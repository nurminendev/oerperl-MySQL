/*

oer+MySQL - IRC bot

See ftp://nic.funet.fi/pub/unix/irc/docs/FAQ.gz section 11 for the
definition of the word bot.

Copyright (C) 2000-2004 EQU <equ@equnet.org>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "oer+MySQL-common.h"
#include "oer+MySQL.h"
#include "parse.h"
#include "ds.h"
#include "misc.h"

/* prototype definitions */
int serverconnection(void);
int registerconnection(void);
int handleserverdata(void);
int processtimeds(void);
int processactions(struct timed *);
void establishconnection(void);
void reconnect(void);
void proxysetup(void);

void establishconnection()
{
#ifdef OER_DEBUG
        oer_debug(OER_DEBUG_NOISE, "establishconnection()\n");
#endif
        /* loop until a fatal error */
        while(!mystate->bailout) {
                mystate->now = time(NULL);
                switch(serverconnection()) {
                case OER_SERVERCONNECTION_ERR_SOCKET:
                        oer_debug(OER_DEBUG_FATAL, "establishconnection->socket() failed, exiting.\n");
                        mystate->bailout = 1;
                        return;
		case OER_SERVERCONNECTION_ERR_GETHOSTNAME:
                        oer_debug(OER_DEBUG_FATAL, "establishconnection->gethostname() failed, exiting.\n");
                        mystate->bailout = 1;
                        return;
                case OER_SERVERCONNECTION_ERR_GETADDRINFO_RETRY:
                        oer_debug(OER_DEBUG_ERROR, "establishconnection->getaddrinfo() temporary error, retrying.\n");
                        sleep(OER_WAIT_BETWEEN_CONNECTION_ATTEMPTS);
                        break;
		case OER_SERVERCONNECTION_ERR_GETSERVER_RETRY:
                        oer_debug(OER_DEBUG_ERROR, "establishconnection->getserver() temporary error, retrying.\n");
                        sleep(OER_WAIT_BETWEEN_CONNECTION_ATTEMPTS);
                        break;
                case OER_SERVERCONNECTION_ERR_GETADDRINFO:
                        oer_debug(OER_DEBUG_FATAL, "establishconnection->getaddrinfo() fatal error, exiting.\n");
                        mystate->bailout = 1;
                        return;
		case OER_SERVERCONNECTION_ERR_BIND:
                        oer_debug(OER_DEBUG_FATAL, "establishconnection->bind() fatal error, exiting.\n");
                        mystate->bailout = 1;
                        return;
                case OER_SERVERCONNECTION_ERR_CONNECT_RETRY:
                        oer_debug(OER_DEBUG_ERROR, "establishconnection->connect() temporary error, retrying.\n");
                        sleep(OER_WAIT_BETWEEN_CONNECTION_ATTEMPTS);
                        break;
                case OER_SERVERCONNECTION_ERR_CONNECT:
                        oer_debug(OER_DEBUG_FATAL, "establishconnection->connect() fatal error, exiting.\n");
                        mystate->bailout = 1;
                        return;
                case OER_SERVERCONNECTION_ALREADY_CONNECTED:
                case OER_SERVERCONNECTION_EVERYTHING_OK:
                        return;
                }
        }
}

void reconnect()
{
	char stringbuffer[WRITE_BUFFER_LENGTH + 1];
#ifdef OER_DEBUG
        oer_debug(OER_DEBUG_NOISE, "reconnect()\n");
#endif
	/* check if connected */
	if(mystate->current_server == NULL) {
		return;
	}
	if(mystate->reconnect == OER_RECONNECT_NONE) {
		return;
	}
	/* this is a workaround for the situation where admin changes oer+MySQL's
           nick to a taken one and then jumps/reconnects, oer+MySQL should at least
           try to get the one the admin specified or fall back to alternative */
        if(strcasecmp(mystate->nick, mystate->getnick)) {
                strncpy(mystate->nick, mystate->getnick, NICKLEN);
        }
	switch(mystate->reconnect) {
	case OER_RECONNECT_ADMIN:
		snprintf(stringbuffer, WRITE_BUFFER_LENGTH, "QUIT :%s\n", mystate->signoff);
#ifdef OER_DEBUG
		oer_debug(OER_DEBUG_INFO, "reconnect->%s", stringbuffer);
#endif
		if(write(mystate->sockfd, stringbuffer, strlen(stringbuffer)) < 0) {
			oer_debug(OER_DEBUG_ERROR, "reconnect->write failed: %s\n", strerror(errno));
		}
		mystate->current_server->tx += strlen(stringbuffer);
		close(mystate->sockfd);
		memset(&mystate->current_server->registered, 0, sizeof(struct registered));
		mystate->current_server->connected = 0;
		mystate->current_server = NULL;
		initall();
#ifdef OER_DEBUG
		oer_debug(OER_DEBUG_INFO, "reconnect->sleeping 30 seconds to avoid throttling\n");
#endif
		sleep(OER_RECONNECT_DELAY);
                establishconnection();
                if(mystate->bailout) {
                        exit(EXIT_FAILURE);
                }
		mystate->reconnect = OER_RECONNECT_NONE;
		break;
	case OER_RECONNECT_STONED:
#ifdef OER_DEBUG
		oer_debug(OER_DEBUG_INFO, "main->current server is stoned, jumping to next one if available\n");
#endif
		close(mystate->sockfd);
		memset(&mystate->current_server->registered, 0, sizeof(struct registered));
		mystate->current_server->connected = 0;
		mystate->current_server = NULL;
		initall();
#ifdef OER_DEBUG
		oer_debug(OER_DEBUG_INFO, "reconnect->sleeping 30 seconds to avoid throttling\n");
#endif
		sleep(OER_RECONNECT_DELAY);
		establishconnection();
                if(mystate->bailout) {
                        exit(EXIT_FAILURE);
                }
		mystate->reconnect = OER_RECONNECT_NONE;
		break;
	case OER_RECONNECT_ERROR:
		if(mystate->quitting) {
#ifdef OER_DEBUG
			oer_debug(OER_DEBUG_INFO, "reconnect->oer+MySQL exiting\n");
#endif
			close(mystate->sockfd);
			mysql_close(&mystate->mysqldb->mysqldbconn);
			exit(EXIT_SUCCESS);
		}
#ifdef OER_DEBUG
		oer_debug(OER_DEBUG_INFO, "reconnect->failed to read/write from/to current server\n");
		oer_debug(OER_DEBUG_INFO, "reconnect->jumping to next server if available\n");
#endif
		close(mystate->sockfd);
                memset(&mystate->current_server->registered, 0, sizeof(struct registered));
		mystate->current_server->connected = 0;
		mystate->current_server = NULL;
		initall();
#ifdef OER_DEBUG
		oer_debug(OER_DEBUG_INFO, "reconnect->sleeping 30 seconds to avoid throttling\n");
#endif
		sleep(OER_RECONNECT_DELAY);
                establishconnection();
                if(mystate->bailout) {
                        exit(EXIT_FAILURE);
                }
		mystate->reconnect = OER_RECONNECT_NONE;
		break;
	}
}

int serverconnection()
{
	char service[TINYSTRINGLEN + 1];
        struct addrinfo *res;
        struct addrinfo *aip;
        struct addrinfo hints;
        int ai_error;
	int sourcefamily;
	int match;
#ifdef OER_DEBUG
        oer_debug(OER_DEBUG_NOISE, "serverconnection()\n");
#endif
        /* check if we are already connected */
        if(mystate->current_server != NULL) {
                if(mystate->current_server->connected) {
                        return OER_SERVERCONNECTION_ALREADY_CONNECTED;
                }
        }
        /* get the system hostname */
        if(gethostname(mystate->host, HOSTLEN) < 0) {
                oer_debug(OER_DEBUG_ERROR, "serverconnection->gethostname() failed: %s\n", strerror(errno));
                return OER_SERVERCONNECTION_ERR_GETHOSTNAME;
        }
	/* assume default hostname is a IPv4 host, if a vhost is specified oer
	   will find out the protocol family for that vhost */
        sourcefamily = PF_INET;
        if(strlen(mystate->vhost)) {
                /* override with virtual host */
                strncpy(mystate->host, mystate->vhost, HOSTLEN);
#ifdef OER_DEBUG
		oer_debug(OER_DEBUG_INFO, "serverconnection->using hostname %s\n", mystate->host);
#endif
                /* bind() is necessary only when we have a vhost */
		memset(&hints, 0, sizeof(hints));
		hints.ai_socktype = SOCK_STREAM;
		if((ai_error = getaddrinfo(mystate->host, NULL, &hints, &res)) != 0) {
			oer_debug(OER_DEBUG_ERROR, "serverconnection->getaddrinfo() for host %s failed: %s (%d)\n", mystate->host, gai_strerror(ai_error), ai_error);
			if(ai_error == EAI_AGAIN) {
				oer_debug(OER_DEBUG_ERROR, "serverconnection->getaddrinfo() non-fatal error, retrying\n");
				return OER_SERVERCONNECTION_ERR_GETADDRINFO_RETRY;
			} else {
				return OER_SERVERCONNECTION_ERR_GETADDRINFO;
			}
		}
		for(aip = res; aip != NULL; aip = aip->ai_next) {
			if((mystate->sockfd = socket(aip->ai_family, aip->ai_socktype, aip->ai_protocol)) < 0) {
				oer_debug(OER_DEBUG_ERROR, "serverconnection->socket() failed: %s\n", strerror(errno));
				freeaddrinfo(res);
				return OER_SERVERCONNECTION_ERR_SOCKET;
			}
			if(bind(mystate->sockfd, aip->ai_addr, aip->ai_addrlen) < 0) {
				oer_debug(OER_DEBUG_ERROR, "serverconnection->bind() for %s failed: %s (check your vhost setting)\n", mystate->host, strerror(errno));
				close(mystate->sockfd);
				freeaddrinfo(res);
				return OER_SERVERCONNECTION_ERR_BIND;
			}
			/* we are satisfied */
                        sourcefamily = aip->ai_family;
			break;
		}
		freeaddrinfo(res);
	}
	/* get IRC server to connect to */
        if((mystate->current_server = getserver()) == NULL) {
                /* we didn't get a server this time but we will eventually
                   once the used & stoned flags are cleared. Unless
                   ofcourse the servers are permanently unreachable. */
                return OER_SERVERCONNECTION_ERR_GETSERVER_RETRY;
        }
        memset(mystate->current_server->serverhost_r, 0, HOSTLEN + 1);
        mystate->current_server->registerconnection_start = mystate->now;
        mystate->current_server->used = 1;
#ifdef OER_DEBUG
        oer_debug(OER_DEBUG_INFO, "serverconnection->using IRC server %s port %d\n", mystate->current_server->serverhost, mystate->current_server->serverport);
#endif
        snprintf(service, TINYSTRINGLEN, "%d", mystate->current_server->serverport);
        memset(&hints, 0, sizeof(hints));
        hints.ai_socktype = SOCK_STREAM;
	if((ai_error = getaddrinfo(mystate->current_server->serverhost, service, &hints, &res)) != 0) {
                oer_debug(OER_DEBUG_ERROR, "serverconnection->getaddrinfo() for host %s service %s failed: %s (%d)\n", mystate->current_server->serverhost, service, gai_strerror(ai_error), ai_error);
		/* if we fail to resolve the server address, always retry */
		return OER_SERVERCONNECTION_ERR_GETADDRINFO_RETRY;
        }
	match = 0;
	for(aip = res; aip != NULL; aip = aip->ai_next) {
		if(aip->ai_family != sourcefamily) {
                        /* conflicting protocol family (IPv4 <-> IPv6), skip */
                        oer_debug(OER_DEBUG_ERROR, "serverconnection->source and destination have different socket families (%d <-> %d), trying next interface/server address\n", sourcefamily, aip->ai_family);
                        continue;
                }
		if(!strlen(mystate->vhost)) {
                        /* we need to create the socket because it wasn't
                           created before for bind() */
			if((mystate->sockfd = socket(aip->ai_family, aip->ai_socktype, aip->ai_protocol)) < 0) {
                                oer_debug(OER_DEBUG_ERROR, "serverconnection->socket() failed: %s\n", strerror(errno));
                                freeaddrinfo(res);
                                return OER_SERVERCONNECTION_ERR_SOCKET;
                        }
                }
                if(connect(mystate->sockfd, aip->ai_addr, aip->ai_addrlen) < 0) {
                        oer_debug(OER_DEBUG_ERROR, "serverconnection->connect() failed: %s (%d)\n", strerror(errno), errno);
                        if(errno == ECONNREFUSED || errno == ETIMEDOUT || errno == ENETUNREACH || errno == EHOSTUNREACH || errno == ECONNRESET) {
#ifdef OER_DEBUG
                                oer_debug(OER_DEBUG_INFO, "serverconnection->connect() non-fatal error\n");
                                oer_debug(OER_DEBUG_INFO, "serverconnection->switching to next server if available\n");
#endif
                                close(mystate->sockfd);
                                freeaddrinfo(res);
                                return OER_SERVERCONNECTION_ERR_CONNECT_RETRY;
                        }
                        close(mystate->sockfd);
                        freeaddrinfo(res);
                        return OER_SERVERCONNECTION_ERR_CONNECT;
                }
                /* we are satisfied */
                match = 1;
                break;
        }
        freeaddrinfo(res);
	if(!match) {
                return OER_SERVERCONNECTION_ERR_SOCKET;
        }
        proxysetup();
        mystate->current_server->connected = 1;
        mystate->current_server->rx = 0;
        mystate->current_server->tx = 0;
        mystate->current_server->linkup = mystate->now;
        return OER_SERVERCONNECTION_EVERYTHING_OK;
}

int registerconnection()
{
	char stringbuffer[WRITE_BUFFER_LENGTH + 1];
	char newuser[USERLEN + 1];
        char prefixes[TINYSTRINGLEN + 1];
	char *ptr;
	struct passwd *pwd;
#ifdef OER_DEBUG
        oer_debug(OER_DEBUG_NOISE, "registerconnection()\n");
#endif
	/* check if connected */
        if(mystate->current_server == NULL) {
                return OER_REGISTERCONNECTION_EVERYTHING_OK;
        }
        /* check that all registration phases are complete */
        if(mystate->current_server->registered.begin
           && mystate->current_server->registered.pass
           && mystate->current_server->registered.nick
           && mystate->current_server->registered.user
           && mystate->current_server->registered.done) {
                return OER_REGISTERCONNECTION_EVERYTHING_OK;
        }
	if((mystate->now - mystate->current_server->registerconnection_start) > OER_REGISTERCONNECTION_TIMEOUT) {
                return OER_REGISTERCONNECTION_TIMEOUT;
        }
	if(!mystate->current_server->registered.begin) {
                /* BEGIN */
#ifdef OER_DEBUG
                oer_debug(OER_DEBUG_INFO, "registerconnection->registering on the server\n");
#endif
                mystate->current_server->registered.begin = 1;
        }
	if(!mystate->current_server->registered.pass) {
                /* PASS, OPTIONAL */
                if(strlen(mystate->current_server->password)) {
                        snprintf(stringbuffer, WRITE_BUFFER_LENGTH, "PASS %s\n", mystate->current_server->password);
#ifdef OER_DEBUG
                        oer_debug(OER_DEBUG_INFO, "registerconnection->%s", stringbuffer);
#endif
                        if(write(mystate->sockfd, stringbuffer, strlen(stringbuffer)) < 0) {
                                oer_debug(OER_DEBUG_ERROR, "registerconnection->write failed: %s\n", strerror(errno));
                                return OER_REGISTERCONNECTION_ERR_WRITE;
                        }
                        mystate->current_server->tx += strlen(stringbuffer);
                }
                mystate->current_server->registered.pass = 1;
        }
	if(!mystate->current_server->registered.nick) {
                /* NICK */
		snprintf(stringbuffer, WRITE_BUFFER_LENGTH, "NICK %s\n", mystate->nick);
#ifdef OER_DEBUG
                oer_debug(OER_DEBUG_INFO, "registerconnection->%s", stringbuffer);
#endif
                if(write(mystate->sockfd, stringbuffer, strlen(stringbuffer)) < 0) {
                        oer_debug(OER_DEBUG_ERROR, "registerconnection->write failed: %s\n", strerror(errno));
                        return OER_REGISTERCONNECTION_ERR_WRITE;
                }
                mystate->current_server->tx += strlen(stringbuffer);
                mystate->current_server->registered.nick = 1;
        }
	if(!mystate->current_server->registered.user) {
                if(!mystate->customuser) {
                        if((pwd = getpwuid(getuid())) == NULL) {
                                return OER_REGISTERCONNECTION_ERR_GETPWUID;
                        }
                        strncpy(mystate->user, pwd->pw_name, USERLEN);
                }
                strncpy(prefixes, CONNECTION_STATE_PREFIXES, TINYSTRINGLEN);
                ptr = mystate->user;
                while(ptr && (index(prefixes, *ptr) != NULL)) {
                        /* strip the prefixes added by the server */
                        ptr++;
                }
                strncpy(newuser, ptr, USERLEN);
                strncpy(mystate->user, newuser, USERLEN);
                snprintf(stringbuffer, WRITE_BUFFER_LENGTH, "USER %s %s %s :%s\n", mystate->user, mystate->host, mystate->current_server->serverhost, mystate->realname);
#ifdef OER_DEBUG
                oer_debug(OER_DEBUG_INFO, "registerconnection->%s", stringbuffer);
#endif
                if(write(mystate->sockfd, stringbuffer, strlen(stringbuffer)) < 0) {
                        oer_debug(OER_DEBUG_ERROR, "registerconnection->write failed: %s\n", strerror(errno));
                        return OER_REGISTERCONNECTION_ERR_WRITE;
                }
                mystate->current_server->tx += strlen(stringbuffer);
                mystate->current_server->registered.user = 1;
        }
        return OER_REGISTERCONNECTION_EVERYTHING_OK;
}

int handleserverdata()
{
	int cont;
	int i;
	char ch;
	char stringbuffer[READ_BUFFER_LENGTH + 1];
        char pong[STRINGLEN + 1];
        size_t br;
        char *ptr;
#ifdef OER_DEBUG
        oer_debug(OER_DEBUG_NOISE, "handleserverdata()\n");
#endif
	i = 0;
	cont = 1;
        do {
                if((br = read(mystate->sockfd, &ch, 1)) < 0) {
			/* error while reading from server socket */
			oer_debug(OER_DEBUG_ERROR, "handleserverdata->read failed: %s\n", strerror(errno));
			return OER_HANDLESERVERDATA_ERR_READ;
                } else if(br == 0) {
			/* nothing or EOF was returned, bail out */
			cont = 0;
		} else if(br > 0 && i < READ_BUFFER_LENGTH) {
			stringbuffer[i++] = ch;
			/* check for newline */
			if(ch == '\n') {
				cont = 0;
			}
		}
	} while(cont);
	if(!i) {
		/* zero bytes was read, bail out */
		return OER_HANDLESERVERDATA_ERR_NOTHING_READ;
	}
	stringbuffer[i] = '\0';
	striplf(stringbuffer);
	/* count only reads from server as server activity, the
           write case to a stoned server will eventually fail in select() */
	mystate->current_server->lastping = mystate->now;
	mystate->current_server->rx += strlen(stringbuffer);
	/* here we log all the server traffic, prefix with timestamp */
	if((index(mystate->flags, (int)'l') != NULL) && (strstr(mystate->state, "+ro") == NULL)) {
		/* it doesn't matter if logging fails */
		mysql_real_escape_string(&mystate->mysqldb->mysqldbconn, mysql_safe_str1, stringbuffer, strlen(stringbuffer));
		oer_doquery(mystate->mysqldb, "handleserverdata", OER_DEBUG_NOISE, "INSERT INTO oer_raw VALUES (%lu, '%s', '%s')", mystate->now, mystate->ident, mysql_safe_str1);
	}
	if((ptr = (char *) strstr(stringbuffer, "PING :")) != NULL) {
		snprintf(pong, STRINGLEN, "PONG :%s\n", ptr + 6);
#ifdef OER_DEBUG
		oer_debug(OER_DEBUG_INFO, "handleserverdata->%s", pong);
#endif
		if(write(mystate->sockfd, pong, strlen(pong)) < 0) {
			oer_debug(OER_DEBUG_ERROR, "handleserverdata->write failed: %s\n", strerror(errno));
			return OER_HANDLESERVERDATA_ERR_WRITE;
		}
		mystate->current_server->tx += strlen(pong);
	}
	else {
		/* everything else but pings */
		parseirc(stringbuffer);
	}
        return OER_HANDLESERVERDATA_EVERYTHING_OK;
}

int processtimeds()
{
        int got_timed;
        struct channel *this;
        struct timed *t1;
        struct timed *t2;
        char stringbuffer[WRITE_BUFFER_LENGTH + 1];
#ifdef OER_DEBUG
        oer_debug(OER_DEBUG_NOISE, "processtimeds()\n");
#endif
        /* check if connected */
        if(mystate->current_server == NULL) {
                return OER_PROCESSTIMEDS_EVERYTHING_OK;
        }
        /* check if registered */
        if(!mystate->current_server->registered.done) {
                return OER_REGISTERCONNECTION_EVERYTHING_OK;
        }
        if(mystate->netjoining) {
                /* don't do any actions while net is joining */
                return OER_PROCESSTIMEDS_EVERYTHING_OK;
        }
        /* lower barrier */
        if(mystate->last_action.ts < mystate->now && mystate->last_action.barrier) {
                mystate->last_action.barrier--;
        }
	for(t1 = mystate->timeds; t1 != NULL; t1 = t2) {
                if(t1->at > mystate->now) {
                        t2 = t1->next;
                        continue;
                }
                /* check if timed can be scheduled now */
                if(!processactions(t1)) {
                        return OER_PROCESSTIMEDS_EVERYTHING_OK;
                }
                got_timed = 0;
                switch(t1->type) {
		case OER_TIMED_TYPE_KICK:
                        if((this = getchptr(t1->channel)) == NULL) {
                                return OER_PROCESSTIMEDS_EVERYTHING_OK;
                        }
                        /* this check is necessary because KICK messages are
                           scheduled before executed and it is possible that
                           there was manual prevention (!unlock) */
                        if(!this->locked.unlocked) {
                                snprintf(stringbuffer, WRITE_BUFFER_LENGTH, "%s\n", t1->command);
                                got_timed = 1;
                        }
                        break;
                case OER_TIMED_TYPE_NORMAL:
                case OER_TIMED_TYPE_USERHOST:
                        snprintf(stringbuffer, WRITE_BUFFER_LENGTH, "%s\n", t1->command);
                        got_timed = 1;
                        break;
                }
		if(got_timed) {
#ifdef OER_DEBUG
                        oer_debug(OER_DEBUG_INFO, "processtimeds->%s", stringbuffer);
#endif
                        if(write(mystate->sockfd, stringbuffer, strlen(stringbuffer)) < 0) {
                                oer_debug(OER_DEBUG_ERROR, "processtimeds->write failed: %s\n", strerror(errno));
                                return OER_PROCESSTIMEDS_ERR_WRITE;
                        }
                        mystate->current_server->tx += strlen(stringbuffer);
                }
                t2 = t1->next;
                timed_del(t1);
                t1 = t2;
        }
        return OER_PROCESSTIMEDS_EVERYTHING_OK;
}

int processactions(struct timed *t)
{
	int delta;
	int weight;
        struct channel *this;

        delta = mystate->now - mystate->last_action.ts;
#ifdef OER_DEBUG
	/* prevent initial flooding of the console */
        if((mystate->now - mystate->current_server->linkup) >= 10) {
		oer_debug(OER_DEBUG_NOISE, "processactions(\"%lu\")\n", t->at);
		oer_debug(OER_DEBUG_INFO, "processactions->last action at %lu / delta %d / barrier %d\n", mystate->last_action.ts, delta, mystate->last_action.barrier);
	}
#endif
	if((t->prio == OER_TIMED_PRIORITY_CHANNEL_HANDLING || t->prio == OER_TIMED_PRIORITY_CHANNEL_PROTECTION) && t->type != OER_TIMED_TYPE_KICK) {
		if(delta || mystate->last_action.barrier < 5) {
			mystate->last_action.ts = mystate->now;
			mystate->last_action.barrier += 1 + (strlen(t->command) / 50);
			return 1;
		}
        }
        if(mystate->last_action.barrier) {
                return 0;
        }
        mystate->last_action.ts = mystate->now;
	switch(t->prio) {
        case OER_TIMED_PRIORITY_WALL:
                if((this = getchptr(t->channel)) == NULL) {
                        return 0;
                }
                /* this is necessary to avoid ERR_TARGETTOOFAST in ircu */
                mystate->last_action.barrier += 5 + (this->nickcount / 2) + (strlen(t->command) / 50);
                break;
        case OER_TIMED_PRIORITY_FLOOD:
                if(t->type == OER_TIMED_TYPE_USERHOST) {
                        mystate->last_action.barrier += 2 + (strlen(t->command) / 50);
                } else {
                        mystate->last_action.barrier += 3 + (strlen(t->command) / 50);
                }
                break;
        case OER_TIMED_PRIORITY_CTCP:
                /* this is a flood protection, somebody could flood an otherwise
                   idle oer by sending it ctcp queries in fixed intervals of 1..2
                   seconds, it all comes down to ircd's flood detection */
                weight = (delta) ? (2 / (delta + 1)) : 2;
                mystate->last_action.barrier += weight + 2 + (strlen(t->command) / 50);
                break;
        case OER_TIMED_PRIORITY_NOTICE:
        case OER_TIMED_PRIORITY_PRIVMSG:
                mystate->last_action.barrier += 1 + (strlen(t->command) / 50);
                break;
        case OER_TIMED_PRIORITY_ADVERT:
                mystate->last_action.barrier += 2 + (strlen(t->command) / 25);
                break;
	case OER_TIMED_PRIORITY_NORMAL:
        case OER_TIMED_PRIORITY_CHANNEL_HANDLING:
        case OER_TIMED_PRIORITY_CHANNEL_PROTECTION:
                if(t->type == OER_TIMED_TYPE_KICK) {
                        mystate->last_action.barrier += 2 + (strlen(t->command) / 50);
                } else {
                        mystate->last_action.barrier += 1 + (strlen(t->command) / 50);
                }
                break;
        }
        return 1;
}

void proxysetup()
{
	char stringbuffer[STRINGLEN + 1];
#ifdef OER_DEBUG
	oer_debug(OER_DEBUG_NOISE, "proxysetup()\n");
#endif
	if(strlen(mystate->proxysetup)) {
		snprintf(stringbuffer, STRINGLEN, "%s\n", mystate->proxysetup);
#ifdef OER_DEBUG
		oer_debug(OER_DEBUG_INFO, "proxysetup->%s", stringbuffer);
#endif
		if(write(mystate->sockfd, stringbuffer, strlen(stringbuffer)) < 0) {
			oer_debug(OER_DEBUG_ERROR, "proxysetup->write failed: %s\n", strerror(errno));
		}
	}
}
