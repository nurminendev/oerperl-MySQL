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
#include "ds.h"
#include "misc.h"
#include "reg.h"
#include "perl.h"

/* prototype definitions */
int checkauth(struct channel *, char *, char *, int, int);
void parseirc(char *);
void parsecommand(char *, char *, char *, char *);
void parsemsg(char *, char *, char *, char *);
void parsenotice(char *, char *, char *, char *);

void parseirc(char *serveroutput)
{
	int i;
	int bans;
	int deops;
	int modetype;
	int ppos;
	int restpos;
	int nrestpos;
	int tpos;
	int do_syncbans;
	int gotallmodes;
	char prefix[STRINGLEN + 1];
	char command[TINYSTRINGLEN + 1];
	char params[STRINGLEN + 1];
        char trailing[WRITE_BUFFER_LENGTH + 1];
	char outstring[STRINGLEN + 1];
	char outstring2[WRITE_BUFFER_LENGTH + 1];
	char outstring3[WRITE_BUFFER_LENGTH + 1];
	char channel[CHANLEN + 1];
	char mode[CHANLEN + 1];
        char modeline[CHANLEN + 1];
	char user[USERHOSTLEN + 1];
	char host[USERHOSTLEN + 1];
	char userhost[USERHOSTLEN + 1];
	/* NICKLEN is not sufficient since sometimes the source/target
           can be a service in which case user@host has to fit into the
           variable defined here */
	char temp_nick[STRINGLEN + 1];
	char temp_nick2[STRINGLEN + 1];
	char target[STRINGLEN + 1];
	char m;
        char *restptr;
        char *ptr;
	struct channel *this;
	struct chanuser *cu;

	striplf(serveroutput);
#ifdef OER_DEBUG
        oer_debug(OER_DEBUG_NOISE, "parseirc(\"%s\")\n", serveroutput);
#endif
        if(serveroutput[0] != ':') {
                /* must be a command */
                return;
        }
        ppos = parseprotocolmessage(serveroutput, prefix, STRINGLEN, command, TINYSTRINGLEN, params, STRINGLEN, trailing, WRITE_BUFFER_LENGTH);
        /* the rest of the serveroutput is serveroutput + ppos */
        restptr = serveroutput + ppos;
#ifdef OER_DEBUG
        oer_debug(OER_DEBUG_NOISE, "parseirc->%s\n", prefix);
        oer_debug(OER_DEBUG_NOISE, "parseirc->%s\n", command);
        oer_debug(OER_DEBUG_NOISE, "parseirc->%s\n", params);
        oer_debug(OER_DEBUG_NOISE, "parseirc->%s\n", trailing);
        oer_debug(OER_DEBUG_NOISE, "parseirc->%s\n", restptr);
#endif
        /* the rest of the line depends on the command/numeric */
	if(!strcasecmp(command, "001")) {
		/* we have registered & are connected to the server */
                mystate->current_server->registered.done = 1;
		/* get the real server name for sstats */
		strncpy(mystate->current_server->serverhost_r, prefix, HOSTLEN);
                /* force a userhost on our nick to find out our hostname */
                snprintf(timed_str, WRITE_BUFFER_LENGTH, "USERHOST %s", mystate->nick);
                timed_new(NULL, mystate->now, OER_TIMED_TYPE_NORMAL, OER_TIMED_PRIORITY_CHANNEL_PROTECTION, timed_str);
				oerperl_call_eventhandlers(prefix, command, params, trailing);
                return;
        }
	if(!strcasecmp(command, "002")) {
		oerperl_call_eventhandlers(prefix, command, params, trailing);
		return;
	}
	if(!strcasecmp(command, "003")) {
		oerperl_call_eventhandlers(prefix, command, params, trailing);
		return;
	}
	if(!strcasecmp(command, "004")) {
		oerperl_call_eventhandlers(prefix, command, params, trailing);
		return;
	}
	if(!strcasecmp(command, "005")) {
		/* at least undernet and dalnet 
		   return the server flags this way */
		oerperl_call_eventhandlers(prefix, command, params, trailing);
		return;
	}
	if(!strcasecmp(command, "221")) {
		oerperl_call_eventhandlers(prefix, command, params, trailing);
		return;
	}
	if(!strcasecmp(command, "250")) {
		/* efnet local clients */
		oerperl_call_eventhandlers(prefix, command, params, trailing);
		return;
	}
	if(!strcasecmp(command, "251")) {
		oerperl_call_eventhandlers(prefix, command, params, trailing);
		return;
	}
	if(!strcasecmp(command, "252")) {
		oerperl_call_eventhandlers(prefix, command, params, trailing);
		return;
	}
	if(!strcasecmp(command, "253")) {
		oerperl_call_eventhandlers(prefix, command, params, trailing);
		return;
	}
	if(!strcasecmp(command, "254")) {
		oerperl_call_eventhandlers(prefix, command, params, trailing);
		return;
	}
	if(!strcasecmp(command, "255")) {
		oerperl_call_eventhandlers(prefix, command, params, trailing);
		return;
	}
	if(!strcasecmp(command, "257")) {
		/* oz.org MOTD extension */
		oerperl_call_eventhandlers(prefix, command, params, trailing);
                return;
        }
	if(!strcasecmp(command, "265")) {
		/* efnet extension, local users */
		oerperl_call_eventhandlers(prefix, command, params, trailing);
		return;
	}
	if(!strcasecmp(command, "266")) {
		/* efnet extension, global users */
		oerperl_call_eventhandlers(prefix, command, params, trailing);
		return;
	}
	if(!strcasecmp(command, "301")) {
                /* this is the response to WHOIS */
		oerperl_call_eventhandlers(prefix, command, params, trailing);
                return;
        }
	if(!strcasecmp(command, "302")) {
                restpos = 0;
                while(1) {
                        nrestpos = parse(trailing, restpos, " ", outstring, STRINGLEN, 0);
                        if(nrestpos == restpos || nrestpos < 0) {
                                break;
                        }
                        restpos = nrestpos;
                        memset(temp_nick, 0, STRINGLEN + 1);
                        tpos = 0;
                        if((tpos = parse(outstring, tpos, "=", temp_nick, STRINGLEN, 1)) < 0) {
							oerperl_call_eventhandlers(prefix, command, params, trailing);
                                return;
                        }
                        if(!strlen(temp_nick)) {
                                /* USERHOST returns emptyline for invalid nicks */
							oerperl_call_eventhandlers(prefix, command, params, trailing);
                                return;
                        }
			/* check for ircop, remove trailing star */
                        if(temp_nick[strlen(temp_nick) - 1] == '*') {
                                temp_nick[strlen(temp_nick) - 1] = '\0';
                                /* isoper */
                                for(this = mystate->channels; this != NULL; this = this->next) {
                                        if(isonchan(this, temp_nick)) {
                                                changeuser(this, temp_nick, 1, -1, -1, -1);
                                        }
                                }
                        }
			strncpy(userhost, outstring + tpos + 1, USERHOSTLEN);
                        if(mystate->qauth.hasauth && !strcasecmp(temp_nick, Q_NICK)) {
                                /* Q query initiated in processenv() */
                                mystate->qauth.isonline = 1;
                        }
                        if(isme(temp_nick)) {
                                /* this is the forced USERHOST query from 001 */
                                strncpy(user, userhost, USERHOSTLEN);
                                ptr = index(user, '@');
                                strncpy(host, ptr + 1, USERHOSTLEN);
                                memset(ptr, 0, strlen(ptr));
                                strncpy(mystate->user, user, USERLEN);
                                strncpy(mystate->host, host, HOSTLEN);
#ifdef OER_DEBUG
                                oer_debug(OER_DEBUG_INFO, "parseirc(302)->the server says I am %s@%s\n", mystate->user, mystate->host);
#endif
                                if(index(mystate->flags, (int)'d') != NULL) {
                                        /* add internal dynamic admin */
                                        if(addnewadmin(OER_INTERNAL_DYNAMIC_ADMIN, "") == NULL) {
#ifdef OER_DEBUG
                                                oer_debug(OER_DEBUG_INFO, "parseirc(302)->failed to add internal dynamic admin %s\n", OER_INTERNAL_DYNAMIC_ADMIN);
#endif
												oerperl_call_eventhandlers(prefix, command, params, trailing);
                                                return;
                                        }
					if(addnewadminmask(OER_INTERNAL_DYNAMIC_ADMIN, userhost) == NULL) {
#ifdef OER_DEBUG
                                                oer_debug(OER_DEBUG_INFO, "parseirc(302)->failed to add mask %s to internal dynamic admin %s\n", userhost, OER_INTERNAL_DYNAMIC_ADMIN);
#endif
												oerperl_call_eventhandlers(prefix, command, params, trailing);
                                                return;
                                        }
#ifdef OER_DEBUG
					oer_debug(OER_DEBUG_INFO, "parseirc(302)->created internal dynamic admin %s with mask %s\n", OER_INTERNAL_DYNAMIC_ADMIN, userhost);
#endif
				}
			} else {
#ifdef OER_DEBUG
				oer_debug(OER_DEBUG_INFO, "parseirc(302)->nick %s has userhost %s\n", temp_nick, userhost);
#endif
				setuserhost(temp_nick, userhost);
			}
		}
		oerperl_call_eventhandlers(prefix, command, params, trailing);
		return;
	}
	if(!strcasecmp(command, "303")) {
		/* this is the response to our ison query */
		oerperl_call_eventhandlers(prefix, command, params, trailing);
		return;
	}
	if(!strcasecmp(command, "311")) {
                /* this is the response to WHOIS */
		oerperl_call_eventhandlers(prefix, command, params, trailing);
                return;
        }
        if(!strcasecmp(command, "312")) {
                /* this is the response to WHOIS */
		oerperl_call_eventhandlers(prefix, command, params, trailing);
                return;
        }
	if(!strcasecmp(command, "313")) {
                /* this is the response to WHOIS */
		oerperl_call_eventhandlers(prefix, command, params, trailing);
                return;
        }
	if(!strcasecmp(command, "315")) {
                /* end of WHO, next query bans */
                if((this = getchptr(trailing)) == NULL) {
					oerperl_call_eventhandlers(prefix, command, params, trailing);
                        return;
                }
                this->whoquery = 1;
		/* make a mode #channel +b query to get all active bans */
                snprintf(timed_str, WRITE_BUFFER_LENGTH, "MODE %s +b", this->name);
                timed_new(NULL, mystate->now, OER_TIMED_TYPE_NORMAL, OER_TIMED_PRIORITY_CHANNEL_HANDLING, timed_str);
				oerperl_call_eventhandlers(prefix, command, params, trailing);
                return;
        }
	if(!strcasecmp(command, "317")) {
                /* this is the response to WHOIS */
		oerperl_call_eventhandlers(prefix, command, params, trailing);
                return;
        }
	if(!strcasecmp(command, "318")) {
                /* this is the response to WHOIS */
		oerperl_call_eventhandlers(prefix, command, params, trailing);
                return;
        }
        if(!strcasecmp(command, "319")) {
                /* returned for WHOIS */
                restpos = 0;
                if((restpos = parse(trailing, restpos, " ", temp_nick, STRINGLEN, 0)) < 0) {
					oerperl_call_eventhandlers(prefix, command, params, trailing);
                        return;
                }
		while(1) {
                        nrestpos = parse(restptr, restpos, " ", channel, CHANLEN, 0);
                        if(nrestpos == restpos || nrestpos < 0) {
                                break;
                        }
                        restpos = nrestpos;
                        if((ptr = index(channel, (int)'#')) == NULL) {
                                /* should never happen */
                                continue;
                        }
                        strncpy(outstring, ptr, STRINGLEN);
                        if((this = getchptr(outstring)) == NULL) {
                                /* it is possible that oer isn't on all of the channels
                                   the queryed nick is on */
                                continue;
                        }
			ptr[0] = '\0';
			/* reset user's flags */
                        changeuser(this, temp_nick, -1, 0, 0, -1);
                        if(index(channel, (int)'@')) {
                                /* @ can hide + -> set @, assume + */
                                changeuser(this, temp_nick, -1, 1, 1, -1);
                        }
                        if(index(channel, (int)'+')) {
                                changeuser(this, temp_nick, -1, -1, 1, -1);
                        }
		}
		oerperl_call_eventhandlers(prefix, command, params, trailing);
                return;
        }
	if(!strcasecmp(command, "324")) {
		if((this = getchptr(trailing)) == NULL) {
			oerperl_call_eventhandlers(prefix, command, params, trailing);
                        return;
                }
                /* channel mode(s) */
                if(getmodepol(restptr, 'k') == '+') {
                        /* avoid 467 */
                        this->chankeyisset = 1;
                } else if(getmodepol(restptr, 'l') == '+') {
                        if(getmodepol(this->mode, 'l') == '+') {
                                /* forced limit, reset to the limit stored */
                                this->chanlimitisset = 0;
                        } else {
                                /* get the dynamicly set limit */
                                tpos = 0;
                                if((tpos = parse(restptr, tpos, " ", mode, CHANLEN, 0)) < 0) {
									oerperl_call_eventhandlers(prefix, command, params, trailing);
                                        return;
                                }
                                /* limit comes first */
                                strncpy(this->limit, restptr + tpos, CHANLEN);
                                this->chanlimitisset = 0;
                        }
                }
				oerperl_call_eventhandlers(prefix, command, params, trailing);
                return;
        }
	if(!strcasecmp(command, "329")) {
	/* channel creation time */
		oerperl_call_eventhandlers(prefix, command, params, trailing);
                return;
        }
	if(!strcasecmp(command, "330")) {
		/* "authed as", this is the response to WHOIS */
		oerperl_call_eventhandlers(prefix, command, params, trailing);
                return;
        }
	if(!strcasecmp(command, "331")) {
		/* the channel has no topic set, returned to a TOPIC command
		   without params (for a channel which has no topic) */
		if((this = getchptr(trailing)) == NULL) {
			oerperl_call_eventhandlers(prefix, command, params, trailing);
			return;
		}
		if(haschanflags(this, "T")) {
			/* skip topic handling for this channel */
			oerperl_call_eventhandlers(prefix, command, params, trailing);
			return;
		}
		/* replace the empty channel topic with ours, if we have one */
		if(topiccount(this)) {
			this->topic_change = 1;
		}
		oerperl_call_eventhandlers(prefix, command, params, trailing);
		return;
	}
	if(!strcasecmp(command, "332")) {
		if((this = getchptr(trailing)) == NULL) {
			oerperl_call_eventhandlers(prefix, command, params, trailing);
			return;
		}
		if(haschanflags(this, "T")) {
			/* skip topic handling for this channel */
			oerperl_call_eventhandlers(prefix, command, params, trailing);
			return;
		}
		if(topiccount(this)) {
			/* only parse topic if oer+MySQL has none for
			   this channel. this is a kludge to avoid empty
			   topics in undernet. we could add a check: 
			   if server's topics differs from ours, we reset
			   it to our's */
			oerperl_call_eventhandlers(prefix, command, params, trailing);
			return;
		}
		if(!gettopic(this, restptr)) {
			oerperl_call_eventhandlers(prefix, command, params, trailing);
			return;
		}
		oerperl_call_eventhandlers(prefix, command, params, trailing);
		return;
	}
	if(!strcasecmp(command, "333")) {
		oerperl_call_eventhandlers(prefix, command, params, trailing);
		return;
	}
	if(!strcasecmp(command, "341")) {
		/* returned when inviting someone */
		oerperl_call_eventhandlers(prefix, command, params, trailing);
		return;
	}
	if(!strcasecmp(command, "352")) {
                /* returned by the WHO query initiated in 366 */
                if((this = getchptr(trailing)) == NULL) {
					oerperl_call_eventhandlers(prefix, command, params, trailing);
                        return;
                }
                restpos = 0;
                if((restpos = parse(restptr, restpos, " ", user, USERHOSTLEN, 0)) < 0) {
					oerperl_call_eventhandlers(prefix, command, params, trailing);
                        return;
                }
                if((restpos = parse(restptr, restpos, " ", host, USERHOSTLEN, 0)) < 0) {
					oerperl_call_eventhandlers(prefix, command, params, trailing);
                        return;
                }
                if((restpos = parse(restptr, restpos, " ", outstring, STRINGLEN, 0)) < 0) {
					oerperl_call_eventhandlers(prefix, command, params, trailing);
                        return;
                }
                if((restpos = parse(restptr, restpos, " ", temp_nick, STRINGLEN, 0)) < 0) {
					oerperl_call_eventhandlers(prefix, command, params, trailing);
                        return;
                }
                if((restpos = parse(restptr, restpos, " ", mode, CHANLEN, 0)) < 0) {
					oerperl_call_eventhandlers(prefix, command, params, trailing);
                        return;
                }
                snprintf(userhost, USERHOSTLEN, "%s@%s", user, host);
                setuserhost(temp_nick, userhost);
		if(index(mode, (int)'@')) {
                        /* @ can hide + -> set @, assume + */
                        changeuser(this, temp_nick, -1, 1, 1, -1);
                }
                if(index(mode, (int)'*')) {
                        changeuser(this, temp_nick, 1, -1, -1, -1);
                }
                if(index(mode, (int)'+')) {
                        changeuser(this, temp_nick, -1, -1, 1, -1);
                }
				oerperl_call_eventhandlers(prefix, command, params, trailing);
                return;
        }
	if(!strcasecmp(command, "353")) {
		restpos = 0;
		if((restpos = parse(restptr, restpos, " ", channel, CHANLEN, 0)) < 0) {
			oerperl_call_eventhandlers(prefix, command, params, trailing);
			return;
		}
		if((this = getchptr(channel)) == NULL) {
			oerperl_call_eventhandlers(prefix, command, params, trailing);
			return;
		}
#ifdef OER_DEBUG
		oer_debug(OER_DEBUG_INFO, "parseirc(353)->%s\n", this->name);
#endif
		if(restptr[restpos] == ':') {
                        restpos++;
                }
		while(1) {
			nrestpos = parse(restptr, restpos, " ", temp_nick, STRINGLEN, 0);
			if(nrestpos == restpos || nrestpos < 0) {
				break;
			}
			restpos = nrestpos;
#ifdef OER_DEBUG
			oer_debug(OER_DEBUG_INFO, "parseirc(353)->%s\n", temp_nick);
#endif
			m = '\0';
			if(temp_nick[0] == '*' || temp_nick[0] == '@' || temp_nick[0] == '+') {
				m = temp_nick[0];
				strncpy(temp_nick2, temp_nick + 1, STRINGLEN);
				strncpy(temp_nick, temp_nick2, STRINGLEN);
			}
			if(userjoined(this, temp_nick, NULL, (m == '*') ? 1 : 0, (m == '@') ? 1 : 0, (m == '+') ? 1 : 0, 0) == NULL) {
#ifdef OER_DEBUG
				oer_debug(OER_DEBUG_INFO, "parseirc(353)->userjoined() returned NULL\n");
#endif
			}
			if(isme(temp_nick) && m == '@') {
				this->i_am_op = 1;
			}
		}
		listchanusers();
		oerperl_call_eventhandlers(prefix, command, params, trailing);
		return;
	}
	if(!strcasecmp(command, "366")) {
		/* 366 is sent after /NAMES, means the join is complete */
		restpos = 0;
		if((this = getchptr(trailing)) == NULL) {
			oerperl_call_eventhandlers(prefix, command, params, trailing);
			return;
		}
#ifdef OER_DEBUG
		oer_debug(OER_DEBUG_INFO, "parseirc(366)->channel %s\n", this->name);
#endif
		this->joined = 1;
		this->joining = 0;
		this->join_ts = mystate->now;
		/* make a WHO query to get all userhosts & flags for
		   nicks on channel */
		snprintf(timed_str, WRITE_BUFFER_LENGTH, "WHO %s", this->name);
		timed_new(NULL, mystate->now, OER_TIMED_TYPE_NORMAL, OER_TIMED_PRIORITY_CHANNEL_HANDLING, timed_str);
		oerperl_call_eventhandlers(prefix, command, params, trailing);
		return;
	}
	if(!strcasecmp(command, "367")) {
		/* channel ban returned from MODE-query */
		if((this = getchptr(trailing)) == NULL) {
			oerperl_call_eventhandlers(prefix, command, params, trailing);
			return;
		}
                /* ban mask */
		restpos = 0;
                if((restpos = parse(restptr, restpos, " ", userhost, USERHOSTLEN, 0)) < 0) {
					oerperl_call_eventhandlers(prefix, command, params, trailing);
                        return;
                }
                if((restpos = parse(restptr, restpos, " ", temp_nick, STRINGLEN, 0)) < 0) {
					oerperl_call_eventhandlers(prefix, command, params, trailing);
                        return;
                }
                /* timestamp */
                if((restpos = parse(restptr, restpos, " ", outstring, STRINGLEN, 0)) < 0) {
					oerperl_call_eventhandlers(prefix, command, params, trailing);
                        return;
                }
                addnewchannelban(this, userhost, temp_nick, atol(outstring));
				oerperl_call_eventhandlers(prefix, command, params, trailing);
                return;
        }
        if(!strcasecmp(command, "368")) {
                /* end of channel bans */
			oerperl_call_eventhandlers(prefix, command, params, trailing);
                return;
        }
	if(!strcasecmp(command, "372")) {
		oerperl_call_eventhandlers(prefix, command, params, trailing);
		return;
	}
	if(!strcasecmp(command, "375")) {
		oerperl_call_eventhandlers(prefix, command, params, trailing);
		return;
	}
	if(!strcasecmp(command, "376")) {
		oerperl_call_eventhandlers(prefix, command, params, trailing);
		return;
	}
	if(!strcasecmp(command, "401")) {
#ifdef OER_DEBUG
		oer_debug(OER_DEBUG_INFO, "parseirc(401)->%s\n", restptr);
#endif
		oerperl_call_eventhandlers(prefix, command, params, trailing);
		return;
	}
	if(!strcasecmp(command, "404")) {
#ifdef OER_DEBUG
		oer_debug(OER_DEBUG_INFO, "parseirc(404)->%s\n", restptr);
#endif
		oerperl_call_eventhandlers(prefix, command, params, trailing);
		return;
	}
	if(!strcasecmp(command, "405")) {
#ifdef OER_DEBUG
                oer_debug(OER_DEBUG_INFO, "parseirc(405)->%s\n", restptr);
#endif
                /* ERR_TOOMANYCHANNELS */
                if((this = getchptr(trailing)) == NULL) {
					oerperl_call_eventhandlers(prefix, command, params, trailing);
                        return;
                }
                /* reset joining flag, we will try again in OER_DELAY_BETWEEN_REJOINS seconds */
                this->rejoin_at = mystate->now + OER_DELAY_BETWEEN_REJOINS;
                this->joining = 0;
				oerperl_call_eventhandlers(prefix, command, params, trailing);
                return;
        }
	if(!strcasecmp(command, "422")) {
#ifdef OER_DEBUG
		oer_debug(OER_DEBUG_INFO, "parseirc(422)->%s\n", restptr);
#endif
		oerperl_call_eventhandlers(prefix, command, params, trailing);
		return;
	}
	if(!strcasecmp(command, "432") || !strcasecmp(command, "433")) {
                /* check if we are still registering */
                if(!mystate->current_server->registered.done) {
                        mystate->current_server->registered.nick = 0;
                        if(mystate->use_altnick) {
#ifdef OER_DEBUG
                                oer_debug(OER_DEBUG_INFO, "parseirc(%s)->both our primary and secondary nicknames are taken, generating a random nick\n", command);
#endif
                                snprintf(mystate->nick, NICKLEN, "oer-%d", getrandom(9999));
                                mystate->use_altnick = 0;
                        } else {
#ifdef OER_DEBUG
                                oer_debug(OER_DEBUG_INFO, "parseirc(%s)->our nickname %s is already in use, will try altnick %s\n", command, trailing, mystate->altnick);
#endif
                                strncpy(mystate->nick, mystate->altnick, NICKLEN);
                                mystate->use_altnick = 1;
                        }
						oerperl_call_eventhandlers(prefix, command, params, trailing);
                        return;
                }
				oerperl_call_eventhandlers(prefix, command, params, trailing);
                return;
        }
	if(!strcasecmp(command, "437")) {
                /* ERR_UNAVAILRESOURCE, handle both cases */
                if((this = getchptr(trailing)) == NULL) {
                        /* nick unavailable, check if we are still registering */
                        if(!mystate->current_server->registered.done) {
                                mystate->current_server->registered.nick = 0;
                                if(mystate->use_altnick) {
#ifdef OER_DEBUG
                                        oer_debug(OER_DEBUG_INFO, "parseirc(%s)->both our primary and secondary nicknames are taken, generating a random nick\n", command);
#endif
                                        snprintf(mystate->nick, NICKLEN, "oer-%d", getrandom(9999));
                                        mystate->use_altnick = 0;
                                } else {
#ifdef OER_DEBUG
                                        oer_debug(OER_DEBUG_INFO, "parseirc(%s)->our nickname %s is already in use, will try altnick %s\n", command, trailing, mystate->altnick);
#endif
                                        strncpy(mystate->nick, mystate->altnick, NICKLEN);
                                        mystate->use_altnick = 1;
                                }
								oerperl_call_eventhandlers(prefix, command, params, trailing);
                                return;
                        }
                        /* no, must be a nick change that failed */
                        strncpy(mystate->nick, params, NICKLEN);
						oerperl_call_eventhandlers(prefix, command, params, trailing);
                        return;
                }
                /* we got channel, reset joining flag */
                this->rejoin_at = mystate->now + OER_DELAY_BETWEEN_REJOINS;
                this->joining = 0;
				oerperl_call_eventhandlers(prefix, command, params, trailing);
                return;
        }
	if(!strcasecmp(command, "439")) {
                if((this = getchptr(trailing)) == NULL) {
					oerperl_call_eventhandlers(prefix, command, params, trailing);
                        return;
                }
                restpos = 0;
                if((ptr = strstr(restptr + restpos, "wait")) == NULL) {
					oerperl_call_eventhandlers(prefix, command, params, trailing);
                        return;
                }
                restpos += 5;
                if((restpos = parse(restptr, restpos, " ", outstring, STRINGLEN, 0)) < 0) {
					oerperl_call_eventhandlers(prefix, command, params, trailing);
                        return;
                }
                i = atoi(outstring);
                if(this->joining && !this->joined && i > 0) {
                        /* reset join status, retry when the server will accept our join */
#ifdef OER_DEBUG
                        oer_debug(OER_DEBUG_INFO, "parseirc(439)->trying to join channel %s again in %d seconds\n", this->name, i);
#endif
                        this->joining = 0;
                        this->rejoin_at = mystate->now + i;
                }
				oerperl_call_eventhandlers(prefix, command, params, trailing);
                return;
        }
	if(!strcasecmp(command, "441")) {
#ifdef OER_DEBUG
		oer_debug(OER_DEBUG_INFO, "parseirc(441)->%s\n", restptr);
#endif
		oerperl_call_eventhandlers(prefix, command, params, trailing);
                return;
        }
	if(!strcasecmp(command, "464")) {
#ifdef OER_DEBUG
                oer_debug(OER_DEBUG_INFO, "parseirc(464)->server reports bad password (%s)\n", (strlen(mystate->current_server->password)) ? mystate->current_server->password : "null");
#endif
                memset(&mystate->current_server->registered, 0, sizeof(struct registered));
				oerperl_call_eventhandlers(prefix, command, params, trailing);
                return;
        }
	if(!strcasecmp(command, "467")) {
		/* channel key already set */
		oerperl_call_eventhandlers(prefix, command, params, trailing);
		return;
	}
	/* couldn't join channel, reset joining flag */
	if(!strcasecmp(command, "471") || !strcasecmp(command, "473") \
	   || !strcasecmp(command, "474") || !strcasecmp(command, "475")) {
		if((this = getchptr(trailing)) == NULL) {
			oerperl_call_eventhandlers(prefix, command, params, trailing);
			return;
		}
		this->rejoin_at = mystate->now + OER_DELAY_BETWEEN_REJOINS;
		this->joining = 0;
		oerperl_call_eventhandlers(prefix, command, params, trailing);
                return;
	}
	if(!strcasecmp(command, "472")) {
		/* unknown channel mode */
		oerperl_call_eventhandlers(prefix, command, params, trailing);
		return;
	}
	if(!strcasecmp(command, "482")) {
		/* you are not op */
		oerperl_call_eventhandlers(prefix, command, params, trailing);
		return;
	}
	if(!strcasecmp(command, "484")) {
		/* undernet extension, protected user */
		restpos = 0;
		if((restpos = parse(restptr, restpos, " ", channel, CHANLEN, 0)) < 0) {
			oerperl_call_eventhandlers(prefix, command, params, trailing);
			return;
		}
		if((this = getchptr(channel)) == NULL) {
			oerperl_call_eventhandlers(prefix, command, params, trailing);
			return;
		}
#ifdef OER_DEBUG
		oer_debug(OER_DEBUG_INFO, "parseirc(484)->nick %s on %s is a IRC operator\n", trailing, this->name);
#endif
		changeuser(this, trailing, 1, -1, -1, -1);
		oerperl_call_eventhandlers(prefix, command, params, trailing);
		return;
	}
	if(!strcasecmp(command, "513")) {
                /* bad ping, this probably means that it took
                   us too long to answer the server ping */
#if 0
                /* oz.org extension, bad bad! */
                mystate->reconnect = OER_RECONNECT_ERROR;
#endif
				oerperl_call_eventhandlers(prefix, command, params, trailing);
                return;
        }
	if(!strcasecmp(command, "JOIN")) {
		if((this = getchptr(params)) == NULL) {
			oerperl_call_eventhandlers(prefix, command, params, trailing);
			return;
		}
		tpos = 0;
		if((tpos = parse(prefix, tpos, "!", temp_nick, STRINGLEN, 1)) < 0) {
			oerperl_call_eventhandlers(prefix, command, params, trailing);
			return;
		}
                if(isme(temp_nick)) {
			/* we are not interested in our own joins, parse channel mode(s) */
                        snprintf(timed_str, WRITE_BUFFER_LENGTH, "MODE %s", this->name);
                        timed_new(NULL, mystate->now, OER_TIMED_TYPE_NORMAL, OER_TIMED_PRIORITY_CHANNEL_PROTECTION, timed_str);
						oerperl_call_eventhandlers(prefix, command, params, trailing);
                        return;
                }
		strncpy(userhost, prefix + tpos, USERHOSTLEN);
		if(userjoined(this, temp_nick, userhost, 0, 0, 0, isfriend(this, temp_nick, userhost)) == NULL) {
#ifdef OER_DEBUG
			oer_debug(OER_DEBUG_INFO, "parseirc(JOIN)->userjoined() returned NULL\n");
#endif
		}
		addnewjoin(this, mystate->now, temp_nick, userhost);
		if(haschanflags(this, "S")) {
			updateseen(this, temp_nick, userhost);
		}
		if(mystate->netjoining) {
			oerperl_call_eventhandlers(prefix, command, params, trailing);
			return;
		}
		if(burstjoins(this) && !mystate->netjoining) {
#ifdef OER_DEBUG
			oer_debug(OER_DEBUG_INFO, "parseirc(JOIN)->net join detected, scheduling netjoin jobs at %lu\n", mystate->now + 30);
#endif
			mystate->netjoining = 1;
			mystate->postnj_checks_at = mystate->now + OER_NETJOIN_DELAY;
			oerperl_call_eventhandlers(prefix, command, params, trailing);
			return;
		}
		listchanusers();
		/* the on-join checks, they require that oer is opped */
                if(this->i_am_op) {
                        if(!gettobek(this, temp_nick) && checkforautorejoin(this, temp_nick, userhost)) {
                                changetobek(this, temp_nick, 1);
                        }
                        if(!gettobek(this, temp_nick) && checkforpartrejoin(this, temp_nick, userhost)) {
                                changetobek(this, temp_nick, 1);
                                resetparts(this, temp_nick, userhost);
                        }
                        if(!gettobek(this, temp_nick) && checkforbadnick(this, temp_nick, userhost)) {
                                changetobek(this, temp_nick, 1);
                        }
			if(!gettobek(this, temp_nick) && haschanflags(this, "V") && isopa(userhost)) {
                                mmode_new(this, mystate->now, "+v", temp_nick);
                        }
			if(!gettobek(this, temp_nick) && haschanflags(this, "v") && isvoice(this, userhost)) {
                                mmode_new(this, mystate->now, "+v", temp_nick);
                        }
                        if(!gettobek(this, temp_nick) && haschanflags(this, "O") && isopa(userhost)) {
				mmode_new(this, mystate->now, "+o", temp_nick);
                        }
                        if(!gettobek(this, temp_nick) && haschanflags(this, "o") && isop(this, userhost)) {
				mmode_new(this, mystate->now, "+o", temp_nick);
                        }
                }
		/* if the user will be kicked, skip the rest */
                if(gettobek(this, temp_nick)) {
					oerperl_call_eventhandlers(prefix, command, params, trailing);
                        return;
                }
                if((mystate->now - this->last_advert) > OER_ADVERT_INTERVAL && haschanflags(this, "A")) {
                        sendadverts(this, temp_nick, userhost);
			this->last_advert = mystate->now;
                }
		if(strstr(mystate->state, "-q") != NULL) {
			oerperl_call_eventhandlers(prefix, command, params, trailing);
			return;
		}
                /* then possibly the on-join quote */
                if(!haschanflags(this, "q")) {
					oerperl_call_eventhandlers(prefix, command, params, trailing);
                        return;
                }
                if(hasuserflags(this, userhost, "x", "")) {
					oerperl_call_eventhandlers(prefix, command, params, trailing);
                        return;
                }
		if((mystate->now - this->last_quote) < OER_QUOTE_INTERVAL) {
			oerperl_call_eventhandlers(prefix, command, params, trailing);
                        return;
                }
                if(getjoincount(this, temp_nick) > 1) {
					oerperl_call_eventhandlers(prefix, command, params, trailing);
                        return;
                }
		strncpy(temp_nick2, temp_nick, STRINGLEN);
		if(getrandommsg(this, temp_nick2, STRINGLEN, outstring2, HUGESTRINGLEN, 0) < 0) {
			oerperl_call_eventhandlers(prefix, command, params, trailing);
			return;
		}
		if(!isgoodquote(outstring2)) {
			oerperl_call_eventhandlers(prefix, command, params, trailing);
			return;
		}
		snprintf(outstring3, HUGESTRINGLEN, "\"%s\" (c) %s", outstring2, temp_nick2);
		sendreply(this->name, 1, 0, OER_TIMED_PRIORITY_NORMAL, outstring3);
		this->last_quote = mystate->now;
		oerperl_call_eventhandlers(prefix, command, params, trailing);
		return;
	}
	if(!strcasecmp(command, "PRIVMSG")) {
		tpos = 0;
                if((tpos = parse(prefix, tpos, "!", temp_nick, STRINGLEN, 1)) < 0) {
					oerperl_call_eventhandlers(prefix, command, params, trailing);
                        return;
                }
                strncpy(userhost, prefix + tpos, USERHOSTLEN);
                parsemsg(temp_nick, userhost, params, trailing);
				oerperl_call_eventhandlers(prefix, command, params, trailing);
                return;
	}
	if(!strcasecmp(command, "QUIT")) {
		tpos = 0;
		if((tpos = parse(prefix, tpos, "!", temp_nick, STRINGLEN, 1)) < 0) {
			oerperl_call_eventhandlers(prefix, command, params, trailing);
			return;
		}
		strncpy(userhost, prefix + tpos, USERHOSTLEN);
		/* quit doesn't provide the channel, we have to check all channels */
		for(this = mystate->channels; this != NULL; this = this->next) {
			if(!isonchan(this, temp_nick)) {
				continue;
			}
			addnewpart(this, mystate->now, temp_nick, userhost);
			if(!userleft(this, temp_nick, userhost)) {
#ifdef OER_DEBUG
				oer_debug(OER_DEBUG_INFO, "parseirc(QUIT)->userleft() returned NULL\n");
#endif
			}
                        initmmodesfornick(this, temp_nick);
			if(mystate->qauth.hasauth && !strcasecmp(temp_nick, Q_NICK)) {
                                mystate->qauth.isonline = 0;
                                mystate->qauth.authed = 0;
                        }
			if(this->nickcount == 1 && !this->i_am_op) {
#ifdef OER_DEBUG
				oer_debug(OER_DEBUG_INFO, "parseirc(QUIT)->cycling %s to gain ops\n", this->name);
#endif
				snprintf(timed_str, WRITE_BUFFER_LENGTH, "PART %s", this->name);
				timed_new(NULL, mystate->now, OER_TIMED_TYPE_NORMAL, OER_TIMED_PRIORITY_CHANNEL_HANDLING, timed_str);
			}
		}
		listchanusers();
		oerperl_call_eventhandlers(prefix, command, params, trailing);
		return;
	}
	if(!strcasecmp(command, "PART")) {
		tpos = 0;
		if((tpos = parse(prefix, tpos, "!", temp_nick, STRINGLEN, 1)) < 0) {
			oerperl_call_eventhandlers(prefix, command, params, trailing);
			return;
		}
		strncpy(userhost, prefix + tpos, USERHOSTLEN);
		if((this = getchptr(params)) == NULL) {
			oerperl_call_eventhandlers(prefix, command, params, trailing);
			return;
		}
		if(isme(temp_nick)) {
			this->rejoin_at = mystate->now + OER_DELAY_BETWEEN_REJOINS;
			if(this->nickcount == 1 && !this->i_am_op) {
				/* we are going to cycle */
				this->rejoin_at = mystate->now + OER_DELAY_BETWEEN_REJOINS_CYCLE;
			}
			/* no reason to remove from channel, we will just init it */
			initchannel(this);
			oerperl_call_eventhandlers(prefix, command, params, trailing);
			return;
		}
		addnewpart(this, mystate->now, temp_nick, userhost);
		if(!userleft(this, temp_nick, userhost)) {
#ifdef OER_DEBUG
			oer_debug(OER_DEBUG_INFO, "parseirc(PART)->userleft() returned NULL\n");
#endif
		}
		initmmodesfornick(this, temp_nick);
		if(this->nickcount == 1 && !this->i_am_op) {
#ifdef OER_DEBUG
			oer_debug(OER_DEBUG_INFO, "parseirc(PART)->cycling %s to gain ops\n", this->name);
#endif
			snprintf(timed_str, WRITE_BUFFER_LENGTH, "PART %s", this->name);
			timed_new(NULL, mystate->now, OER_TIMED_TYPE_NORMAL, OER_TIMED_PRIORITY_CHANNEL_HANDLING, timed_str);
		}
		listchanusers();
		oerperl_call_eventhandlers(prefix, command, params, trailing);
		return;
	}
	if(!strcasecmp(command, "NICK")) {
                tpos = 0;
                if((tpos = parse(prefix, tpos, "!", temp_nick, STRINGLEN, 1)) < 0) {
					oerperl_call_eventhandlers(prefix, command, params, trailing);
                        return;
                }
                strncpy(userhost, prefix + tpos, USERHOSTLEN);
		if(isme(temp_nick)) {
                        /* oer+MySQL changed it's nick either because it was
                         asked to or it wasn't the primary nick and
			 global flag "g" was set */
#ifdef OER_DEBUG
                        oer_debug(OER_DEBUG_INFO, "nickchange->I changed my nick from %s to %s\n", temp_nick, params);
#endif
                        strncpy(mystate->nick, params, NICKLEN);
                }
#ifdef OER_DEBUG
                oer_debug(OER_DEBUG_INFO, "nickchange->%s is now known as %s, changing on all channels\n", temp_nick, params);
#endif
                /* if user changes nick while we are doing HOSTNAME queries
                   on channel join, we will end up with userhost = NULL for the user */
                if(ishostless(temp_nick) == 1) {
                        sethostquerystatus(temp_nick, 0);
                }
                /* nick doesn't provide the channel, we have to check all channels */
		for(this = mystate->channels; this != NULL; this = this->next) {
                        if(isonchan(this, temp_nick)) {
                                addnewnickchange(this, userhost);
                                /* if a user tries to avoid getting kicked
                                   on !lock by changing nick, we will adapt */
                                changetobek(this, temp_nick, 0);
                                if(this->i_am_op && checkforbadnick(this, params, userhost)) {
                                        changetobek(this, temp_nick, 1);
                                }
                                if(this->i_am_op && checkfornickflood(this, temp_nick, userhost)) {
					changetobek(this, temp_nick, 1);
				}
                                nickchange(this, temp_nick, params);
				changemmodesfornick(this, temp_nick, params);
                        }
                }
		oerperl_call_eventhandlers(prefix, command, params, trailing);
                return;
        }
	if(!strcasecmp(command, "KILL")) {
		if(isme(params)) {
#ifdef OER_DEBUG
			oer_debug(OER_DEBUG_INFO, "parseirc(KILL)->I was killed by %s path %s\n", prefix, trailing);
#endif
		}
		oerperl_call_eventhandlers(prefix, command, params, trailing);
		return;
	}
	if(!strcasecmp(command, "KICK")) {
		tpos = 0;
		if((tpos = parse(prefix, tpos, "!", temp_nick, STRINGLEN, 1)) < 0) {
			oerperl_call_eventhandlers(prefix, command, params, trailing);
			return;
		}
		strncpy(userhost, prefix + tpos, USERHOSTLEN);
		if((this = getchptr(params)) == NULL) {
			oerperl_call_eventhandlers(prefix, command, params, trailing);
			return;
		}
		if(isme(trailing)) {
#ifdef OER_DEBUG
			oer_debug(OER_DEBUG_INFO, "parseirc(KICK)->I was kicked off %s by %s!%s\n", this->name, temp_nick, userhost);
#endif
			initchannel(this);
			this->rejoin_at = mystate->now + OER_DELAY_BETWEEN_REJOINS;
			oerperl_call_eventhandlers(prefix, command, params, trailing);
			return;
		}
		/* KICK doesn't tell the host for the person that was kicked */
		if((cu = getcuptr(this, trailing)) == NULL) {
			/* is a bogus/lagged server/service messages */
			oerperl_call_eventhandlers(prefix, command, params, trailing);
			return;
		}
#ifdef OER_DEBUG
		oer_debug(OER_DEBUG_INFO, "parseirc(KICK)->%s!%s off %s by %s!%s\n", trailing, (cu->userhost == NULL) ? "(null)" : cu->userhost, this->name, temp_nick, userhost);
#endif
		addnewpart(this, mystate->now, trailing, cu->userhost);
		if(!userleft(this, trailing, cu->userhost)) {
#ifdef OER_DEBUG
			oer_debug(OER_DEBUG_INFO, "parseirc(KICK)->userleft() returned NULL\n");
#endif
		}
		initmmodesfornick(this, trailing);
		if(this->nickcount == 1 && !this->i_am_op) {
#ifdef OER_DEBUG
			oer_debug(OER_DEBUG_INFO, "parseirc(QUIT)->cycling %s to gain ops\n", this->name);
#endif
			snprintf(timed_str, WRITE_BUFFER_LENGTH, "PART %s", this->name);
			timed_new(NULL, mystate->now, OER_TIMED_TYPE_NORMAL, OER_TIMED_PRIORITY_CHANNEL_HANDLING, timed_str);
		}
		listchanusers();
		oerperl_call_eventhandlers(prefix, command, params, trailing);
		return;
	}
	if(!strcasecmp(command, "NOTICE")) {
		/* handle the sync notice sent
		   by another oer+MySQL instance in a shared setup */
		tpos = 0;
		if((tpos = parse(prefix, tpos, "!", temp_nick, STRINGLEN, 1)) < 0) {
			oerperl_call_eventhandlers(prefix, command, params, trailing);
			return;
		}
		strncpy(userhost, prefix + tpos, USERHOSTLEN);
		parsenotice(temp_nick, userhost, params, trailing);
		oerperl_call_eventhandlers(prefix, command, params, trailing);
		return;
	}
	if(!strcasecmp(command, "MODE")) {
		tpos = 0;
                if((tpos = parse(prefix, tpos, "!", temp_nick, STRINGLEN, 1)) < 0) {
					oerperl_call_eventhandlers(prefix, command, params, trailing);
                        return;
                }
                if(strlen(prefix + tpos)) {
                        strncpy(userhost, prefix + tpos, USERHOSTLEN);
                } else {
                        memset(userhost, 0, USERHOSTLEN + 1);
                }
                if(isme(params)) {
#ifdef OER_DEBUG
                        oer_debug(OER_DEBUG_INFO, "parseirc(MODE)->usermode is %s\n", trailing);
#endif
						oerperl_call_eventhandlers(prefix, command, params, trailing);
                        return;
                }
                if((this = getchptr(params)) == NULL) {
					oerperl_call_eventhandlers(prefix, command, params, trailing);
                        return;
                }
                restpos = 0;
                /* channel modes have no target */
		if(strlen(restptr)) {
#ifdef OER_DEBUG
                        oer_debug(OER_DEBUG_INFO, "parseirc(MODE)->%s %s on %s by %s!%s\n", trailing, restptr, this->name, temp_nick, userhost);
#endif
                } else {
#ifdef OER_DEBUG
                        oer_debug(OER_DEBUG_INFO, "parseirc(MODE)->%s on %s by %s!%s\n", trailing, this->name, temp_nick, userhost);
#endif
                }
		for(i = 0, gotallmodes = 0, bans = 0, deops = 0, do_syncbans = 0; !gotallmodes; i++) {
                        switch((modetype = nthmode(trailing, i))) {
                        case OER_NTHMODE_OP:
                        case OER_NTHMODE_DOP:
                        case OER_NTHMODE_VOICE:
                        case OER_NTHMODE_DVOICE:
                        case OER_NTHMODE_BAN:
                        case OER_NTHMODE_UNBAN:
                        case OER_NTHMODE_CHANMODE_WITH_PARAMS:
				nrestpos = parse(restptr, restpos, " ", target, STRINGLEN, 0);
                                if((nrestpos != restpos) && (nrestpos > 0)) {
                                        restpos = nrestpos;
                                        getnthmode(trailing, i + 1, modeline);
                                        oer_debug(OER_DEBUG_INFO, "parseirc(MODE)->modeline is %s\n", modeline);
                                        if(!isme(temp_nick)) {
                                                /* the following are only necessary for
                                                   manual user actions, check channel key */
                                                if(!strcasecmp(modeline, "+k")) {
                                                        strncpy(this->key, target, CHANLEN);
                                                        if(getmodepol(this->mode, 'k') != '-') {
                                                                /* avoid 467 */
                                                                this->chankeyisset = 1;
                                                        }
                                                } else if(!strcasecmp(modeline, "-k")) {
                                                        if(getmodepol(this->mode, 'k') != '+') {
                                                                memset(this->key, 0, CHANLEN + 1);
                                                        } else {
                                                                /* reset */
                                                                this->chankeyisset = 0;
                                                        }
                                                }
						/* check channel limit (+l) */
                                                if(!strcasecmp(modeline, "+l")) {
                                                        strncpy(this->limit, target, CHANLEN);
                                                        if(getmodepol(this->mode, 'l') != '-') {
                                                                /* avoid re-setting */
                                                                this->chanlimitisset = 1;
                                                        } else {
                                                                this->chanlimitisset = 0;
                                                        }
                                                }
                                        }
#ifdef OER_DEBUG
                                        oer_debug(OER_DEBUG_INFO, "parseirc(MODE)->target is %s\n", target);
#endif
                                } else {
                                        gotallmodes = 1;
                                }
                                break;
			case OER_NTHMODE_CHANMODE_WITHOUT_PARAMS:
                                getnthmode(trailing, i + 1, modeline);
                                oer_debug(OER_DEBUG_INFO, "parseirc(MODE)->modeline is %s\n", modeline);
                                if(!isme(temp_nick)) {
                                        /* check channel limit (-l) */
                                        if(!strcasecmp(modeline, "-l")) {
                                                if(getmodepol(this->mode, 'l') != '+') {
                                                        memset(this->limit, 0, CHANLEN + 1);
                                                } else {
                                                        /* reset */
                                                        this->chanlimitisset = 0;
                                                }
                                        }
                                }
                                break;
                        case OER_NTHMODE_UNKNOWN:
                                gotallmodes = 1;
                                break;
                        }
			switch(modetype) {
                        case OER_NTHMODE_OP:
#ifdef OER_DEBUG
                                oer_debug(OER_DEBUG_NOISE, "parseirc(MODE)->OER_NTHMODE_OP\n");
#endif
                                changeuser(this, target, -1, 1, -1, -1);
                                if(isme(target)) {
                                        this->i_am_op = 1;
                                        if(this->allhostsknown) {
                                                channelsync(this);
                                        }
                                        continue;
                                }
                                if(!this->i_am_op) {
                                        continue;
                                }
                                if(haschanflags(this, "u") && !isallowedop(this, target)) {
                                        if(isq(temp_nick) || isservice(temp_nick) || hasuserflags(this, userhost, "s", "")) {
                                                /* allow Q/service/special user to op */
                                                continue;
                                        }
#ifdef OER_DEBUG
                                        oer_debug(OER_DEBUG_NOISE, "parseirc(MODE)->non-allowed op of nick %s on channel %s by %s!%s\n", target, this->name, temp_nick, userhost);
#endif
                                        mmode_new(this, mystate->now, "-o", target);
                                }
                                break;
			case OER_NTHMODE_DOP:
#ifdef OER_DEBUG
                                oer_debug(OER_DEBUG_NOISE, "parseirc(MODE)->OER_NTHMODE_DOP\n");
#endif
                                changeuser(this, target, -1, 0, -1, -1);
                                /* user lost @ -> really check for + */
                                snprintf(timed_str, WRITE_BUFFER_LENGTH, "WHOIS %s", target);
                                timed_new(NULL, mystate->now, OER_TIMED_TYPE_NORMAL, OER_TIMED_PRIORITY_CHANNEL_PROTECTION, timed_str);
                                /* check if the last user on channel was de-opped, if
                                   so activate requestop */
                                if(ischanopless(this)) {
                                        this->requestop_now = 1;
                                        this->requestop = mystate->now + 300;
                                }
                                if(isme(target)) {
                                        this->i_am_op = 0;
                                }
                                if(isq(temp_nick) || isservice(temp_nick) || hasuserflags(this, userhost, "s", "")) {
                                        /* allow Q/service/special user to de-op */
                                        continue;
                                }
                                if(!this->i_am_op) {
                                        continue;
                                }
                                if(haschanflags(this, "D") && isallowedop(this, target)) {
                                        /* re-op */
                                        mmode_new(this, mystate->now, "+o", target);
                                }
                                deops++;
                                break;
			case OER_NTHMODE_VOICE:
#ifdef OER_DEBUG
                                oer_debug(OER_DEBUG_NOISE, "parseirc(MODE)->OER_NTHMODE_VOICE\n");
#endif
                                changeuser(this, target, -1, -1, 1, -1);
                                break;
                        case OER_NTHMODE_DVOICE:
#ifdef OER_DEBUG
                                oer_debug(OER_DEBUG_NOISE, "parseirc(MODE)->OER_NTHMODE_DVOICE\n");
#endif
                                changeuser(this, target, -1, -1, 0, -1);
                                break;
			case OER_NTHMODE_BAN:
#ifdef OER_DEBUG
                                oer_debug(OER_DEBUG_NOISE, "parseirc(MODE)->OER_NTHMODE_BAN\n");
#endif
                                if(strlen(userhost)) {
                                        snprintf(outstring, STRINGLEN, "%s!%s", temp_nick, userhost)
						;
                                } else {
                                        strncpy(outstring, temp_nick, STRINGLEN);
                                }
                                addnewchannelban(this, target, outstring, mystate->now);
                                snprintf(outstring, STRINGLEN, "%s!%s@%s", mystate->nick, mystate->user, mystate->host);
				if(wild_match(target, outstring)) {
#ifdef OER_DEBUG
                                        oer_debug(OER_DEBUG_INFO, "parseirc(MODE)->ban detected on me (%s)\n", target);
#endif
                                        if(isq(temp_nick) || isservice(temp_nick) || hasuserflags(this, userhost, "s", "")) {
                                                /* allow Q/service/special user to ban us */
                                                continue;
                                        }
                                        if(this->i_am_op && haschanflags(this, "b")) {
                                                /* i am being banned, first kick then unban */
                                                if(!isme(temp_nick) && !isserver(temp_nick)) {
                                                        snprintf(timed_str, WRITE_BUFFER_LENGTH, "KICK %s %s :do NOT ban %s, that's me!", this->name, temp_nick, target);
                                                        timed_new(NULL, mystate->now, OER_TIMED_TYPE_NORMAL, OER_TIMED_PRIORITY_CHANNEL_PROTECTION, timed_str);
                                                }
                                                mmode_new(this, mystate->now, "-b", target);
                                        }
                                }
                                bans++;
                                break;
			case OER_NTHMODE_UNBAN:
#ifdef OER_DEBUG
                                oer_debug(OER_DEBUG_NOISE, "parseirc(MODE)->OER_NTHMODE_UNBAN\n");
#endif
                                delchannelban(this, target);
                                if(this->i_am_op && haschanflags(this, "e") && ispermban(this, target)) {
                                        if(isq(temp_nick) || isservice(temp_nick) || hasuserflags(this, userhost, "s", "")) {
                                                /* allow Q/service/special user to unban */
                                                continue;
                                        }
                                        /* re-set the ban */
#ifdef OER_DEBUG
                                        oer_debug(OER_DEBUG_INFO, "parseirc(MODE)->%s on channel %s tried to remove the permban %s\n", temp_nick, this->name, target);
#endif
                                        mmode_new(this, mystate->now, "+b", target);
                                        do_syncbans = 1;
                                }
                                break;
			case OER_NTHMODE_CHANMODE_WITHOUT_PARAMS:
#ifdef OER_DEBUG
                                oer_debug(OER_DEBUG_NOISE, "parseirc(MODE)->OER_NTHMODE_CHANMODE_WITHOUT_PARAMS\n");
#endif
                                if(!isme(temp_nick)) {
                                        this->setchanmode = 1;
                                }
                                break;
                        case OER_NTHMODE_CHANMODE_WITH_PARAMS:
#ifdef OER_DEBUG
                                oer_debug(OER_DEBUG_NOISE, "parseirc(MODE)->OER_NTHMODE_CHANMODE_WITH_PARAMS\n");
#endif
                                if(!isme(temp_nick)) {
                                        this->setchanmode = 1;
                                }
                                break;
                        case OER_NTHMODE_UNKNOWN:
                                gotallmodes = 1;
                                break;
                        }
			if((deops > OER_MASSMODE_LIMIT) && (index(userhost, '@') != NULL) && !isme(temp_nick)) {
#ifdef OER_DEBUG
                                oer_debug(OER_DEBUG_INFO, "parseirc(MODE)->mass modes detected on %s from %s\n", this->name, temp_nick);
#endif
                                if(isq(temp_nick) || isservice(temp_nick) || hasuserflags(this, userhost, "s", "")) {
                                        /* allow Q/service/special user to do massmodes */
                                        continue;
                                }
                                if(this->i_am_op && haschanflags(this, "m") && !isserver(temp_nick)) {
                                        mmode_new(this, mystate->now, "-o", temp_nick);
                                        sendreply(temp_nick, 0, 3, OER_TIMED_PRIORITY_NORMAL, "do NOT deop >3 users at a time");
                                        deops = 0;
                                        bans = 0;
                                }
                        }
                }
                if(do_syncbans) {
                        /* sync bans. if a user has set ban *@*.se and there is a
                           permban *@*.swipnet.se, the latter has to be re-insert when
                           the former is manually removed */
                        syncpermbans(this);
                }
				oerperl_call_eventhandlers(prefix, command, params, trailing);
                return;
        }
	if(!strcasecmp(command, "TOPIC")) {
                tpos = 0;
                if((tpos = parse(prefix, tpos, "!", temp_nick, STRINGLEN, 1)) < 0) {
					oerperl_call_eventhandlers(prefix, command, params, trailing);
                        return;
                }
                if((this = getchptr(params)) == NULL) {
					oerperl_call_eventhandlers(prefix, command, params, trailing);
                        return;
                }
                if(haschanflags(this, "T")) {
                        /* skip topic handling for this channel */
					oerperl_call_eventhandlers(prefix, command, params, trailing);
                        return;
                }
                if(haschanflags(this, "t")) {
			/* only allow topic changes through us and services */
                        if(!isme(temp_nick) && !isq(temp_nick) && !isservice(temp_nick)) {
                                this->topic_change = 1;
                        }
                }
				oerperl_call_eventhandlers(prefix, command, params, trailing);
                return;
        }
	if(!strcasecmp(command, "INVITE")) {
		tpos = 0;
                if((tpos = parse(prefix, tpos, "!", temp_nick, STRINGLEN, 1)) < 0) {
					oerperl_call_eventhandlers(prefix, command, params, trailing);
                        return;
                }
                strncpy(userhost, prefix + tpos, USERHOSTLEN);
                if((this = getchptr(trailing)) == NULL) {
					oerperl_call_eventhandlers(prefix, command, params, trailing);
                        return;
                }
#ifdef OER_DEBUG
                oer_debug(OER_DEBUG_INFO, "parseirc(INVITE)->%s invited me to %s\n", temp_nick, this->name);
#endif
				oerperl_call_eventhandlers(prefix, command, params, trailing);
                return;
	}
#ifdef OER_DEBUG
	oer_debug(OER_DEBUG_INFO, "parseirc(UNKNOWN)->%s\n", prefix);
	oer_debug(OER_DEBUG_INFO, "parseirc(UNKNOWN)->%s\n", command);
	oer_debug(OER_DEBUG_INFO, "parseirc(UNKNOWN)->%s\n", params);
	oer_debug(OER_DEBUG_INFO, "parseirc(UNKNOWN)->%s\n", restptr);
#endif
}

void parsemsg(char *nick, char *userhost, char *target, char *message)
{
	int ctcp;
	int ppos;
	int is_command;
	struct channel *this;
	char outstring[WRITE_BUFFER_LENGTH + 1];
#ifdef OER_DEBUG
        oer_debug(OER_DEBUG_INFO, "parsemsg(\"%s\", \"%s\", \"%s\", \"%s\")\n", nick, userhost, target, message);
#endif
	ctcp = 0;
	if(isctcp(message)) {
		ctcp = whichctcp(message);
		switch(ctcp) {
		case OER_WHICHCTCP_ACTION:
			if(isme(target)) {
				/* bogus action */
				return;
			}
#ifdef OER_DEBUG
			oer_debug(OER_DEBUG_INFO, "parsemsg->isctcp = 1 and ctcp = ACTION\n");
#endif
			break;
		case OER_WHICHCTCP_FINGER:
#ifdef OER_DEBUG
			oer_debug(OER_DEBUG_INFO, "parsemsg->isctcp = 1 and ctcp = FINGER\n");
#endif
			parsectcp(nick, userhost, ctcp, message);
			break;
		case OER_WHICHCTCP_PING:
#ifdef OER_DEBUG
			oer_debug(OER_DEBUG_INFO, "parsemsg->isctcp = 1 and ctcp = PING\n");
#endif
			parsectcp(nick, userhost, ctcp, message);
			break;
		case OER_WHICHCTCP_USERINFO:
#ifdef OER_DEBUG
			oer_debug(OER_DEBUG_INFO, "parsemsg->isctcp = 1 and ctcp = USERINFO\n");
#endif
			parsectcp(nick, userhost, ctcp, message);
			break;
		case OER_WHICHCTCP_VERSION:
#ifdef OER_DEBUG
			oer_debug(OER_DEBUG_INFO, "parsemsg->isctcp = 1 and ctcp = VERSION\n");
#endif
			parsectcp(nick, userhost, ctcp, message);
			break;
		case OER_WHICHCTCP_INVALID:
#ifdef OER_DEBUG
			oer_debug(OER_DEBUG_INFO, "parsemsg->invalid CTCP from %s!%s target: %s\n", nick, userhost, target);
#endif
			break;
		}
	}
	if(ctcp && isme(target)) {
		/* we have already handled all personal CTCP messages */
		return;
	}
	if(isme(target)) {
                /* the message is sent to me and not to a channel */
                parsecommand(target, nick, userhost, message);
                return;
        }
	if((this = getchptr(target)) == NULL) {
		return;
	}
	/* the message is a public message sent to a channel */
	if(this->i_am_op && checkforansi(this, nick, userhost, message)) {
		return;
	}
	if(this->i_am_op && checkforflood(this, nick, userhost, message)) {
		return;
	}
	if(this->i_am_op && checkforbadword(this, nick, userhost, message)) {
		return;
	}
	if(this->i_am_op && checkforaction(this, nick, userhost, ctcp)) {
                return;
        }
	/* only add this consequetive line iff the user
	   isn't being kicked already (lag between kick and last message) */
	if(!gettobek(this, nick)) {
		addnewpubmsg(this, mystate->now, nick, userhost, message);
	}
	if(ctcp) {
		/* we don't log ctcp (not even ACTIONS) */
		return;
	}
	/* check for command */
        ppos = 0;
        is_command = 0;
	if((ppos = parse(message, ppos, " ", outstring, WRITE_BUFFER_LENGTH, 0)) >= 0) {
                if(!strcasecmp(outstring, mystate->nick)) {
                        is_command = 1;
                } else if(!strncasecmp(outstring, mystate->prefix, strlen(mystate->prefix))) {
                        is_command = 1;
                        ppos = strlen(mystate->prefix);
                }
        }
	if(is_command && !hasuserflags(this, userhost, "n", "")) {
                /* the message is sent to a channel -> parse */
                parsecommand(this->name, nick, userhost, message + ppos);
        }
	/* it is possible that the channel was deleted */
	if((this = getchptr(target)) == NULL) {
#ifdef OER_DEBUG
		oer_debug(OER_DEBUG_INFO, "parsemsg->channel %s was deleted\n", target);
#endif
		return;
	}
	if(hasuserflags(this, userhost, "x", "")) {
		/* user's messages won't be saved */
#ifdef OER_DEBUG
		oer_debug(OER_DEBUG_INFO, "parsemsg->messages from %s to %s won't be saved\n", nick, this->name);
#endif
		return;
	}
	/* strip mIRC colors */
	stripmirc(message);
	/* strip all control chars from message */
	stripcntrl(message);
	/* update the channel last information */
	if(haschanflags(this, "L")) {
		updatelast(this, nick, userhost, message);
	}
}

void parsecommand(char *target, char *nick, char *userhost, char *commandline)
{
	int which;
	int tochan;
        int counter;
	int ppos;
	int nppos;
        int prevppos;
	int numofchans;
        int numofadmins;
        int numofusers;
        int numofchanusers;
        int numofadminmasks;
        int numofusermasks;
	int numofoppedchanusers;
        int numofvoicedchanusers;
        int numofopers;
	int numofmmodes;
        int numoftimeds;
        int i_am_voiced;
	time_t uptime_seconds;
	time_t idle_seconds;
        time_t idle_per;
        char p1[STRINGLEN + 1];
        char p2[STRINGLEN + 1];
        char p3[STRINGLEN + 1];
	char outstring[WRITE_BUFFER_LENGTH + 1];
	char outstring2[WRITE_BUFFER_LENGTH + 1];
	char command[STRINGLEN + 1];
	char channel[CHANLEN + 1];
	char uptimestring[STRINGLEN + 1];
	char modeline[CHANLEN + 1];
	char ts[TINYSTRINGLEN + 1];
	char *restptr;
	char *ptr;
	struct channel *this;
	struct channel *that;
	struct chanuser *u;
	struct botuser *admin;
	struct botuser *user;
	struct channelban *cb;
	struct maskstruct *ms;
#ifdef OER_DEBUG
        oer_debug(OER_DEBUG_INFO, "parsecommand(\"%s\", \"%s\", \"%s\", \"%s\")\n", target, nick, userhost, commandline);
#endif
	/* command line parsing begins here, first parameter = the command */
        ppos = 0;
        if((ppos = parse(commandline, ppos, " ", command, STRINGLEN, 0)) < 0) {
                return;
        }
	prevppos = ppos;
	/* second is either the channel or first parameter to the command */
	if((ppos = parse(commandline, ppos, " ", outstring, WRITE_BUFFER_LENGTH, 0)) < 0) {
                return;
        }
	tochan = (isme(target)) ? 0 : 1;
        if(!tochan) {
		if(!isvalidchannel(outstring)) {
                        /* no channel given, continue from here */
                        strncpy(channel, "<NONE>", CHANLEN);
                        restptr = commandline + prevppos;
                } else {
                        /* advance to next parameter, skip channel */
                        strncpy(channel, outstring, CHANLEN);
                        restptr = commandline + ppos;
                }
        } else {
                strncpy(channel, target, CHANLEN);
                restptr = commandline + prevppos;
        }
#ifdef OER_DEBUG
	oer_debug(OER_DEBUG_INFO, "parsecommand->command is %s and channel is %s\n", command, channel);
#endif
	if(strlen(restptr) > 0) {
#ifdef OER_DEBUG
		oer_debug(OER_DEBUG_INFO, "parsecommand->rest of the commandline is %s\n", restptr);
#endif
	}
	switch((which = whichcommand(command, wordcount(restptr)))) {
	case OER_WHICHCOMMAND_TOPIC_SET:
#ifdef OER_DEBUG
		oer_debug(OER_DEBUG_INFO, "parsecommand->TOPIC_SET\n");
#endif
		if(oerperl_call_commandhandlers(command, restptr, target, nick, userhost, 1) == OERPERL_BLOCK_OER_COMMAND) {
			return;
		}
                if((this = getchptr(channel)) == NULL) {
                        return;
                }
                if(!this->i_am_op) {
                        return;
                }
                if(haschanflags(this, "T")) {
                        /* skip topic handling for this channel */
                        return;
                }
                if(!checkauth(this, userhost, nick, 1, 0)) {
                        return;
                }
		if(!setnewtopic(this, nick, restptr)) {
			snprintf(outstring, WRITE_BUFFER_LENGTH, "setnewtopic() failed");
			sendreply((tochan) ? channel : nick, tochan, 0, OER_TIMED_PRIORITY_NORMAL, outstring);
		}
		return;
	case OER_WHICHCOMMAND_TOPIC_ADD:
#ifdef OER_DEBUG
		oer_debug(OER_DEBUG_INFO, "parsecommand->TOPIC_ADD\n");
#endif
		if(oerperl_call_commandhandlers(command, restptr, target, nick, userhost, 1) == OERPERL_BLOCK_OER_COMMAND) {
			return;
		}
                if((this = getchptr(channel)) == NULL) {
                        return;
                }
                if(!this->i_am_op) {
                        return;
                }
                if(haschanflags(this, "T")) {
                        /* skip topic handling for this channel */
                        return;
                }
                if(!checkauth(this, userhost, nick, 1, 0)) {
                        return;
                }
		if(!addnewtopic(this, nick, restptr)) {
			snprintf(outstring, WRITE_BUFFER_LENGTH, "addnewtopic() failed");
			sendreply((tochan) ? channel : nick, tochan, 0, OER_TIMED_PRIORITY_NORMAL, outstring);
		}
		return;
	case OER_WHICHCOMMAND_TOPIC_DEL:
#ifdef OER_DEBUG
		oer_debug(OER_DEBUG_INFO, "parsecommand->TOPIC_DEL\n");
#endif
		if(oerperl_call_commandhandlers(command, restptr, target, nick, userhost, 1) == OERPERL_BLOCK_OER_COMMAND) {
			return;
		}
                if((this = getchptr(channel)) == NULL) {
                        return;
                }
                if(!this->i_am_op) {
                        return;
                }
                if(haschanflags(this, "T")) {
                        /* skip topic handling for this channel */
                        return;
                }
                if(!checkauth(this, userhost, nick, 1, 0)) {
                        return;
                }
		if(!deltopic(this, (tochan) ? this->name : nick, tochan, nick, userhost, restptr)) {
			snprintf(outstring, WRITE_BUFFER_LENGTH, "deltopic() failed");
			sendreply((tochan) ? channel : nick, tochan, 0, OER_TIMED_PRIORITY_NORMAL, outstring);
		}
		return;
	case OER_WHICHCOMMAND_TOPIC_EDIT:
#ifdef OER_DEBUG
		oer_debug(OER_DEBUG_INFO, "parsecommand->TOPIC_EDIT\n");
#endif
		if(oerperl_call_commandhandlers(command, restptr, target, nick, userhost, 1) == OERPERL_BLOCK_OER_COMMAND) {
			return;
		}
                if((this = getchptr(channel)) == NULL) {
                        return;
                }
                if(!this->i_am_op) {
                        return;
                }
                if(haschanflags(this, "T")) {
                        /* skip topic handling for this channel */
                        return;
                }
                if(!checkauth(this, userhost, nick, 1, 0)) {
                        return;
                }
		ppos = 0;
		if((ppos = parse(restptr, ppos, " ", p1, STRINGLEN, 0)) < 0) {
			return;
		}
		if(!strlen(p1) || !strlen(restptr + ppos) || !this->i_am_op) {
			return;
		}
		if(!edittopic(this, nick, restptr + ppos, atoi(p1))) {
			snprintf(outstring, WRITE_BUFFER_LENGTH, "edittopic() failed");
			sendreply((tochan) ? channel : nick, tochan, 0, OER_TIMED_PRIORITY_NORMAL, outstring);
		}
		return;
	case OER_WHICHCOMMAND_TOPIC_INS:
#ifdef OER_DEBUG
		oer_debug(OER_DEBUG_INFO, "parsecommand->TOPIC_INS\n");
#endif
		if(oerperl_call_commandhandlers(command, restptr, target, nick, userhost, 1) == OERPERL_BLOCK_OER_COMMAND) {
			return;
		}
                if((this = getchptr(channel)) == NULL) {
                        return;
                }
                if(!this->i_am_op) {
                        return;
                }
                if(haschanflags(this, "T")) {
                        /* skip topic handling for this channel */
                        return;
                }
                if(!checkauth(this, userhost, nick, 1, 0)) {
                        return;
                }
		ppos = 0;
		if((ppos = parse(restptr, ppos, " ", p1, STRINGLEN, 0)) < 0) {
			return;
		}
		if(!strlen(p1) || !strlen(restptr + ppos) || !this->i_am_op) {
			return;
		}
		if(!insertnewtopic(this, nick, restptr + ppos, atoi(p1))) {
			snprintf(outstring, WRITE_BUFFER_LENGTH, "insertnewtopic() failed");
			sendreply((tochan) ? channel : nick, tochan, 0, OER_TIMED_PRIORITY_NORMAL, outstring);
		}
		return;
	case OER_WHICHCOMMAND_TOPIC_LIST:
#ifdef OER_DEBUG
		oer_debug(OER_DEBUG_INFO, "parsecommand->TOPIC_LIST\n");
#endif
		if(oerperl_call_commandhandlers(command, restptr, target, nick, userhost, 1) == OERPERL_BLOCK_OER_COMMAND) {
			return;
		}
		if((this = getchptr(channel)) == NULL) {
                        return;
                }
                if(haschanflags(this, "T")) {
                        /* skip topic handling for this channel */
                        return;
                }
                if(!checkauth(this, userhost, nick, 1, 0)) {
                        return;
                }
		listtopic(this, (tochan) ? this->name : nick, tochan);
		return;
	case OER_WHICHCOMMAND_TOPIC_GET:
#ifdef OER_DEBUG
		oer_debug(OER_DEBUG_INFO, "parsecommand->TOPIC_GET\n");
#endif
		if(oerperl_call_commandhandlers(command, restptr, target, nick, userhost, 1) == OERPERL_BLOCK_OER_COMMAND) {
			return;
		}
		if((this = getchptr(channel)) == NULL) {
                        return;
                }
                if(haschanflags(this, "T")) {
                        /* skip topic handling for this channel */
                        return;
                }
                if(!checkauth(this, userhost, nick, 1, 0)) {
                        return;
                }
		if(!deltopics(this)) {
			return;
		}
		snprintf(timed_str, WRITE_BUFFER_LENGTH, "TOPIC %s", channel);
		timed_new(NULL, mystate->now, OER_TIMED_TYPE_NORMAL, OER_TIMED_PRIORITY_CHANNEL_HANDLING, timed_str);
		return;
	case OER_WHICHCOMMAND_TOPIC_REFRESH:
#ifdef OER_DEBUG
		oer_debug(OER_DEBUG_INFO, "parsecommand->TOPIC_REFRESH\n");
#endif
		if(oerperl_call_commandhandlers(command, restptr, target, nick, userhost, 1) == OERPERL_BLOCK_OER_COMMAND) {
			return;
		}
		if((this = getchptr(channel)) == NULL) {
                        return;
                }
                if(!this->i_am_op) {
                        return;
                }
                if(haschanflags(this, "T")) {
                        /* skip topic handling for this channel */
                        return;
                }
                if(!checkauth(this, userhost, nick, 1, 0)) {
                        return;
                }
		this->topic_change = 1;
		return;
	case OER_WHICHCOMMAND_TOPIC_SWAP:
#ifdef OER_DEBUG
		oer_debug(OER_DEBUG_INFO, "parsecommand->TOPIC_SWAP\n");
#endif
		if(oerperl_call_commandhandlers(command, restptr, target, nick, userhost, 1) == OERPERL_BLOCK_OER_COMMAND) {
			return;
		}
                if((this = getchptr(channel)) == NULL) {
                        return;
                }
                if(!this->i_am_op) {
                        return;
                }
                if(haschanflags(this, "T")) {
                        /* skip topic handling for this channel */
                        return;
                }
                if(!checkauth(this, userhost, nick, 1, 0)) {
                        return;
                }
		ppos = 0;
		if((ppos = parse(restptr, ppos, " ", p1, STRINGLEN, 0)) < 0) {
			return;
		}
		if((ppos = parse(restptr, ppos, " ", p2, STRINGLEN, 0)) < 0) {
			return;
		}
		if(!strlen(p1) || !strlen(p2) || !this->i_am_op) {
			return;
		}
		if(!swaptopic(this, atoi(p1), atoi(p2))) {
			snprintf(outstring, WRITE_BUFFER_LENGTH, "swaptopic() failed");
			sendreply((tochan) ? channel : nick, tochan, 0, OER_TIMED_PRIORITY_NORMAL, outstring);
		}
		return;
	case OER_WHICHCOMMAND_LOCK:
	case OER_WHICHCOMMAND_LOCKU:
#ifdef OER_DEBUG
		oer_debug(OER_DEBUG_INFO, "parsecommand->LOCK/LOCKU\n");
#endif
		if(oerperl_call_commandhandlers(command, restptr, target, nick, userhost, 1) == OERPERL_BLOCK_OER_COMMAND) {
			return;
		}
		if((this = getchptr(channel)) == NULL) {
                        return;
                }
                if(!this->i_am_op) {
                        return;
                }
                /* allow admins to lock even if not "l" chanflag */
                if(!isadmin(this, userhost)) {
                        if(!haschanflags(this, "l")) {
                                return;
                        }
                        if(!checkauth(this, userhost, nick, 1, 0)) {
                                return;
                        }
                }
                lockchan(this, restptr, (which == OER_WHICHCOMMAND_LOCK) ? 0 : 1, nick, userhost);
		return;
	case OER_WHICHCOMMAND_UNLOCK:
#ifdef OER_DEBUG
		oer_debug(OER_DEBUG_INFO, "parsecommand->UNLOCK\n");
#endif
		if(oerperl_call_commandhandlers(command, restptr, target, nick, userhost, 1) == OERPERL_BLOCK_OER_COMMAND) {
			return;
		}
		if((this = getchptr(channel)) == NULL) {
                        return;
                }
                if(!this->i_am_op) {
                        return;
                }
                if(!checkauth(this, userhost, nick, 1, 0)) {
                        return;
                }
		unlockchan(this, nick, userhost);
		return;
	case OER_WHICHCOMMAND_RANDOM_KICK:
#ifdef OER_DEBUG
		oer_debug(OER_DEBUG_INFO, "parsecommand->RANDOM_KICK\n");
#endif
		if(oerperl_call_commandhandlers(command, restptr, target, nick, userhost, 1) == OERPERL_BLOCK_OER_COMMAND) {
			return;
		}
		if((this = getchptr(channel)) == NULL) {
                        return;
                }
                if(!this->i_am_op) {
                        return;
                }
                /* allow admins to use !rk even if they don't have "r" user flag */
                if(!isadmin(this, userhost)) {
                        if(!hasuserflags(this, userhost, "or", "")) {
                                return;
                        }
                }
		if((u = getrandomuser(this)) == NULL) {
			return;
		}
		snprintf(outstring, WRITE_BUFFER_LENGTH, "random kick by %s", nick);
		kickuser(this, mystate->now, u->nick, outstring);
		return;
	case OER_WHICHCOMMAND_RANDOM_BANKICK:
#ifdef OER_DEBUG
                oer_debug(OER_DEBUG_INFO, "parsecommand->RANDOM_BANKICK\n");
#endif
		if(oerperl_call_commandhandlers(command, restptr, target, nick, userhost, 1) == OERPERL_BLOCK_OER_COMMAND) {
			return;
		}
		if((this = getchptr(channel)) == NULL) {
                        return;
                }
                if(!this->i_am_op) {
                        return;
                }
                /* allow admins to use !rbk even if they don't have "r" user flag */
                if(!isadmin(this, userhost)) {
                        if(!hasuserflags(this, userhost, "or", "")) {
                                return;
                        }
                }
		if((u = getrandomuser(this)) == NULL) {
			return;
		}
		snprintf(outstring, WRITE_BUFFER_LENGTH, "random ban kick by %s", nick);
		banuser(this, mystate->now - 1, u->nick);
		kickuser(this, mystate->now, u->nick, outstring);
                return;
	case OER_WHICHCOMMAND_SYNC:
#ifdef OER_DEBUG
		oer_debug(OER_DEBUG_INFO, "parsecommand->SYNC\n");
#endif
		if(oerperl_call_commandhandlers(command, restptr, target, nick, userhost, 1) == OERPERL_BLOCK_OER_COMMAND) {
			return;
		}
		if((this = getchptr(channel)) == NULL) {
                        return;
                }
                if(!this->i_am_op) {
                        return;
                }
                if(!checkauth(this, userhost, nick, 1, 0)) {
                        return;
                }
		syncvoices(this);
		syncops(this);
		syncnickbks(this);
		syncpermbans(this);
		return;
	case OER_WHICHCOMMAND_SYNCALL:
#ifdef OER_DEBUG
		oer_debug(OER_DEBUG_INFO, "parsecommand->SYNCALL\n");
#endif
		if(oerperl_call_commandhandlers(command, restptr, target, nick, userhost, 1) == OERPERL_BLOCK_OER_COMMAND) {
			return;
		}
		if(!isopa(userhost)) {
			return;
		}
		for(that = mystate->channels; that != NULL; that = that->next) {
			if(!that->i_am_op) {
				continue;
			}
			syncvoices(that);
			syncops(that);
			syncnickbks(that);
			syncpermbans(that);
		}
		return;
	case OER_WHICHCOMMAND_JUMP:
#ifdef OER_DEBUG
		oer_debug(OER_DEBUG_INFO, "parsecommand->JUMP\n");
#endif
		if(oerperl_call_commandhandlers(command, restptr, target, nick, userhost, 1) == OERPERL_BLOCK_OER_COMMAND) {
			return;
		}
		if(!isopa(userhost)) {
			return;
		}
		if(!strlen(restptr)) {
			snprintf(mystate->signoff, STRINGLEN, "jump to next IRC server ordered by %s (%s)", nick, userhost);
		} else {
			if(!isvalidhost(restptr)) {
				return;
			}
			snprintf(mystate->signoff, STRINGLEN, "jump to server %s ordered by %s (%s)", restptr, nick, userhost);
			strncpy(mystate->preferredserver, restptr, HOSTLEN);
		}
		mystate->reconnect = OER_RECONNECT_ADMIN;
		return;
	case OER_WHICHCOMMAND_QUIT:
#ifdef OER_DEBUG
		oer_debug(OER_DEBUG_INFO, "parsecommand->QUIT\n");
#endif
		if(oerperl_call_commandhandlers(command, restptr, target, nick, userhost, 1) == OERPERL_BLOCK_OER_COMMAND) {
			return;
		}
		restptr = commandline + prevppos;
		if(!isopa(userhost)) {
			return;
		}
		if(!strlen(restptr)) {
			snprintf(mystate->signoff, STRINGLEN, "quit ordered by %s (%s)", nick, userhost);
		} else {
			strncpy(mystate->signoff, restptr, STRINGLEN);
		}
		quit();
		return;
	case OER_WHICHCOMMAND_OP:
#ifdef OER_DEBUG
		oer_debug(OER_DEBUG_INFO, "parsecommand->OP\n");
#endif
		if(oerperl_call_commandhandlers(command, restptr, target, nick, userhost, 1) == OERPERL_BLOCK_OER_COMMAND) {
			return;
		}
		if((this = getchptr(channel)) == NULL) {
                        return;
                }
                if(!this->i_am_op) {
                        return;
                }
                if(!checkauth(this, userhost, nick, 1, 0)) {
                        return;
                }
		if(!strlen(restptr)) {
			/* the messager wants to be opped */
			if(!isonchan(this, nick) || isopnow(this, nick)) {
				return;
			}
			if(haschanflags(this, "u") && !isallowedop(this, nick)) {
				return;
			}
			mmode_new(this, mystate->now, "+o", nick);
			return;
		}
		/* op all given nicks */
		ppos = 0;
		while(1) {
			nppos = parse(restptr, ppos, " ", p1, STRINGLEN, 0);
			if(nppos == ppos || nppos < 0) {
				break;
			}
			ppos = nppos;
			if(!isonchan(this, p1) || isopnow(this, p1)) {
				continue;
			}
			if(haschanflags(this, "u") && !isallowedop(this, p1)) {
				continue;
			}
			mmode_new(this, mystate->now, "+o", p1);
		}
		return;
	case OER_WHICHCOMMAND_DOP:
#ifdef OER_DEBUG
		oer_debug(OER_DEBUG_INFO, "parsecommand->DOP\n");
#endif
		if(oerperl_call_commandhandlers(command, restptr, target, nick, userhost, 1) == OERPERL_BLOCK_OER_COMMAND) {
			return;
		}
		if((this = getchptr(channel)) == NULL) {
                        return;
                }
                if(!this->i_am_op) {
                        return;
                }
                if(!checkauth(this, userhost, nick, 1, 0)) {
                        return;
                }
		if(!strlen(restptr)) {
			/* the messager wants to be de-opped */
			if(!isonchan(this, nick) || !isopnow(this, nick)) {
				return;
			}
			if(haschanflags(this, "D") && isallowedop(this, nick)) {
				return;
			}
			mmode_new(this, mystate->now, "-o", nick);
			return;
		}
		/* de-op all given nicks */
		ppos = 0;
		while(1) {
			nppos = parse(restptr, ppos, " ", p1, STRINGLEN, 0);
			if(nppos == ppos || nppos < 0) {
				break;
			}
			ppos = nppos;
			if(!isonchan(this, p1) || !isopnow(this, p1)) {
				continue;
			}
			if(haschanflags(this, "D") && isallowedop(this, p1)) {
				continue;
			}
			mmode_new(this, mystate->now, "-o", p1);
		}
		return;
	case OER_WHICHCOMMAND_VOICE:
#ifdef OER_DEBUG
		oer_debug(OER_DEBUG_INFO, "parsecommand->VOICE\n");
#endif
		if(oerperl_call_commandhandlers(command, restptr, target, nick, userhost, 1) == OERPERL_BLOCK_OER_COMMAND) {
			return;
		}
		if((this = getchptr(channel)) == NULL) {
                        return;
                }
                if(!this->i_am_op) {
                        return;
                }
                if(!checkauth(this, userhost, nick, 1, 0)) {
                        return;
                }
		if(!strlen(restptr)) {
			/* the messager wants to be voiced */
			if(isonchan(this, nick) && !isvoicenow(this, nick)) {
                                mmode_new(this, mystate->now, "+v", nick);
			}
			return;
		}
		/* voice all given nicks */
		ppos = 0;
		while(1) {
			nppos = parse(restptr, ppos, " ", p1, STRINGLEN, 0);
			if(nppos == ppos || nppos < 0) {
				break;
			}
			ppos = nppos;
			if(isonchan(this, p1) && !isvoicenow(this, p1)) {
                                mmode_new(this, mystate->now, "+v", p1);
			}
		}
		return;
	case OER_WHICHCOMMAND_DEVOICE:
#ifdef OER_DEBUG
		oer_debug(OER_DEBUG_INFO, "parsecommand->DEVOICE\n");
#endif
		if(oerperl_call_commandhandlers(command, restptr, target, nick, userhost, 1) == OERPERL_BLOCK_OER_COMMAND) {
			return;
		}
		if((this = getchptr(channel)) == NULL) {
                        return;
                }
                if(!this->i_am_op) {
                        return;
                }
                if(!checkauth(this, userhost, nick, 1, 0)) {
                        return;
                }
		if(!strlen(restptr)) {
			/* the messager wants to be devoiced */
			if(isonchan(this, nick) && isvoicenow(this, nick)) {
                                mmode_new(this, mystate->now, "-v", nick);
			}
			return;
		}
		/* devoice all given nicks */
		ppos = 0;
		while(1) {
			nppos = parse(restptr, ppos, " ", p1, STRINGLEN, 0);
			if(nppos == ppos || nppos < 0) {
				break;
			}
			ppos = nppos;
			if(isonchan(this, p1) && isvoicenow(this, p1)) {
                                mmode_new(this, mystate->now, "-v", p1);
			}
		}
		return;
	case OER_WHICHCOMMAND_BAN:
#ifdef OER_DEBUG
		oer_debug(OER_DEBUG_INFO, "parsecommand->BAN\n");
#endif
		if(oerperl_call_commandhandlers(command, restptr, target, nick, userhost, 1) == OERPERL_BLOCK_OER_COMMAND) {
			return;
		}
                if((this = getchptr(channel)) == NULL) {
                        return;
                }
                if(!this->i_am_op) {
                        return;
                }
                if(!checkauth(this, userhost, nick, 1, 0)) {
                        return;
                }
		/* ban all given nicks */
		ppos = 0;
		while(1) {
			nppos = parse(restptr, ppos, " ", p1, STRINGLEN, 0);
			if(nppos == ppos || nppos < 0) {
				break;
			}
			ppos = nppos;
			banuser(this, mystate->now, p1);
		}
		return;
	case OER_WHICHCOMMAND_UNBAN:
#ifdef OER_DEBUG
		oer_debug(OER_DEBUG_INFO, "parsecommand->UNBAN\n");
#endif
		if(oerperl_call_commandhandlers(command, restptr, target, nick, userhost, 1) == OERPERL_BLOCK_OER_COMMAND) {
			return;
		}
                if((this = getchptr(channel)) == NULL) {
                        return;
                }
                if(!this->i_am_op) {
                        return;
                }
                if(!checkauth(this, userhost, nick, 1, 0)) {
                        return;
                }
		/* unban all given nicks */
		ppos = 0;
		while(1) {
			nppos = parse(restptr, ppos, " ", p1, STRINGLEN, 0);
			if(nppos == ppos || nppos < 0) {
				break;
			}
			ppos = nppos;
			unbanuser(this, mystate->now, p1);
		}
		return;
	case OER_WHICHCOMMAND_BANKICK:
#ifdef OER_DEBUG
		oer_debug(OER_DEBUG_INFO, "parsecommand->BANKICK\n");
#endif
		if(oerperl_call_commandhandlers(command, restptr, target, nick, userhost, 1) == OERPERL_BLOCK_OER_COMMAND) {
			return;
		}
                if((this = getchptr(channel)) == NULL) {
                        return;
                }
                if(!this->i_am_op) {
                        return;
                }
                if(!checkauth(this, userhost, nick, 1, 0)) {
                        return;
                }
		ppos = 0;
		if((ppos = parse(restptr, ppos, " ", p1, STRINGLEN, 0)) < 0) {
			return;
		}
		banuser(this, mystate->now - 1, p1);
		kickuser(this, mystate->now, p1, (strlen(restptr + ppos) > 0) ? restptr + ppos : NULL);
		return;
	case OER_WHICHCOMMAND_KICK:
#ifdef OER_DEBUG
		oer_debug(OER_DEBUG_INFO, "parsecommand->KICK\n");
#endif
		if(oerperl_call_commandhandlers(command, restptr, target, nick, userhost, 1) == OERPERL_BLOCK_OER_COMMAND) {
			return;
		}
                if((this = getchptr(channel)) == NULL) {
                        return;
                }
                if(!this->i_am_op) {
                        return;
                }
                if(!checkauth(this, userhost, nick, 1, 0)) {
                        return;
                }
		ppos = 0;
		if((ppos = parse(restptr, ppos, " ", p1, STRINGLEN, 0)) < 0) {
			return;
		}
		kickuser(this, mystate->now, p1, (strlen(restptr + ppos) > 0) ? restptr + ppos : NULL);
		return;
	case OER_WHICHCOMMAND_INVITE:
#ifdef OER_DEBUG
		oer_debug(OER_DEBUG_INFO, "parsecommand->INVITE\n");
#endif
		if(oerperl_call_commandhandlers(command, restptr, target, nick, userhost, 1) == OERPERL_BLOCK_OER_COMMAND) {
			return;
		}
		if(tochan) {
                        return;
                }
                if((this = getchptr(channel)) == NULL) {
                        return;
                }
                if(!this->i_am_op) {
                        return;
                }
                if(!checkauth(this, userhost, nick, 1, 0)) {
                        return;
                }
		snprintf(timed_str, WRITE_BUFFER_LENGTH, "INVITE %s %s", nick, channel);
		timed_new(NULL, mystate->now, OER_TIMED_TYPE_NORMAL, OER_TIMED_PRIORITY_CHANNEL_HANDLING, timed_str);
		return;
	case OER_WHICHCOMMAND_MASSMESSAGE:
#ifdef OER_DEBUG
		oer_debug(OER_DEBUG_INFO, "parsecommand->MASSMESSAGE\n");
#endif
		if(oerperl_call_commandhandlers(command, restptr, target, nick, userhost, 1) == OERPERL_BLOCK_OER_COMMAND) {
			return;
		}
		restptr = commandline + prevppos;
		if(!isopa(userhost)) {
			return;
		}
		if(strlen(restptr) > 0) {
			massmessage(nick, restptr);
		}
		return;
	case OER_WHICHCOMMAND_MODE:
#ifdef OER_DEBUG
		oer_debug(OER_DEBUG_INFO, "parsecommand->MODE\n");
#endif
		if(oerperl_call_commandhandlers(command, restptr, target, nick, userhost, 1) == OERPERL_BLOCK_OER_COMMAND) {
			return;
		}
                if((this = getchptr(channel)) == NULL) {
                        return;
                }
                if(!this->i_am_op) {
                        return;
                }
                if(!checkauth(this, userhost, nick, 1, 0)) {
                        return;
                }
		ppos = 0;
		if((ppos = parse(restptr, ppos, " ", p1, STRINGLEN, 0)) < 0) {
			return;
		}
		if(!strlen(p1)) {
			return;
		}
		if(!strlen(restptr + ppos)) {
			/* only a mode line, no target */
			mmode_new(this, mystate->now, p1, NULL);
			return;
		}
		/* process all modes */
		counter = 1;
		while(1) {
			nppos = parse(restptr, ppos, " ", p2, STRINGLEN, 0);
			if(nppos == ppos || nppos < 0) {
				break;
			}
			ppos = nppos;
			getnthmode(p1, counter, modeline);
#ifdef OER_DEBUG
			oer_debug(OER_DEBUG_INFO, "parsecommand->mode %s %s %s\n", channel, modeline, p2);
#endif
			mmode_new(this, mystate->now, modeline, p2);
			counter++;
		}
		return;
	case OER_WHICHCOMMAND_HELP:
#ifdef OER_DEBUG
		oer_debug(OER_DEBUG_INFO, "parsecommand->HELP\n");
#endif
		if(oerperl_call_commandhandlers(command, restptr, target, nick, userhost, 1) == OERPERL_BLOCK_OER_COMMAND) {
			return;
		}
		/* channel is optional */
                this = getchptr(channel);
                if(!checkauth(this, userhost, nick, tochan, 1)) {
                        return;
                }
		strncpy(outstring, OER_HELP, WRITE_BUFFER_LENGTH);
		sendreply((tochan) ? channel : nick, tochan, 0, OER_TIMED_PRIORITY_NORMAL, outstring);
		return;
	case OER_WHICHCOMMAND_LIST:
#ifdef OER_DEBUG
		oer_debug(OER_DEBUG_INFO, "parsecommand->LIST\n");
#endif
		ppos = 0;
		if((ppos = parse(restptr, ppos, " ", p1, STRINGLEN, 0)) < 0) {
			return;
		}
		/* list scripts cannot be overridden by scripts */
		if(strcasecmp(p1, "scripts") != 0) {
			if(oerperl_call_commandhandlers(command, restptr, target, nick, userhost, 1) == OERPERL_BLOCK_OER_COMMAND) {
				return;
			}
		}
                /* channel is optional */
                this = getchptr(channel);
                if(!checkauth(this, userhost, nick, tochan, 0)) {
                        return;
                }
		listcommand(this, (tochan) ? this->name : nick, tochan, nick, userhost, restptr);
		return;
	case OER_WHICHCOMMAND_ADD:
#ifdef OER_DEBUG
		oer_debug(OER_DEBUG_INFO, "parsecommand->ADD\n");
#endif
		ppos = 0;
		if((ppos = parse(restptr, ppos, " ", p1, STRINGLEN, 0)) < 0) {
			return;
		}
		/* add script cannot be overridden by scripts */
		if(strcasecmp(p1, "script") != 0) {
			if(oerperl_call_commandhandlers(command, restptr, target, nick, userhost, 1) == OERPERL_BLOCK_OER_COMMAND) {
				return;
			}
		}
		/* channel is optional */
                this = getchptr(channel);
                if(!checkauth(this, userhost, nick, tochan, 0)) {
                        return;
                }
		addcommand(this, (tochan) ? this->name : nick, tochan, nick, userhost, restptr);
		return;
	case OER_WHICHCOMMAND_DEL:
#ifdef OER_DEBUG
		oer_debug(OER_DEBUG_INFO, "parsecommand->DEL\n");
#endif
		ppos = 0;
		if((ppos = parse(restptr, ppos, " ", p1, STRINGLEN, 0)) < 0) {
			return;
		}
		/* del script cannot be overridden by scripts */
		if(strcasecmp(p1, "script") != 0) {
			if(oerperl_call_commandhandlers(command, restptr, target, nick, userhost, 1) == OERPERL_BLOCK_OER_COMMAND) {
				return;
			}
		}
                /* channel is optional */
                this = getchptr(channel);
                if(!checkauth(this, userhost, nick, tochan, 0)) {
                        return;
                }
		delcommand(this, (tochan) ? this->name : nick, tochan, nick, userhost, restptr);
		return;
	case OER_WHICHCOMMAND_EDIT:
#ifdef OER_DEBUG
		oer_debug(OER_DEBUG_INFO, "parsecommand->EDIT\n");
#endif
		if(oerperl_call_commandhandlers(command, restptr, target, nick, userhost, 1) == OERPERL_BLOCK_OER_COMMAND) {
			return;
		}
                /* channel is optional */
                this = getchptr(channel);
                if(!checkauth(this, userhost, nick, tochan, 0)) {
                        return;
                }
		editcommand(this, (tochan) ? this->name : nick, tochan, nick, userhost, restptr);
		return;
	case OER_WHICHCOMMAND_LOGOFF:
#ifdef OER_DEBUG
		oer_debug(OER_DEBUG_INFO, "parsecommand->LOGOFF\n");
#endif
		if(oerperl_call_commandhandlers(command, restptr, target, nick, userhost, 1) == OERPERL_BLOCK_OER_COMMAND) {
			return;
		}
                /* channel is optional */
                this = getchptr(channel);
		logoffcommand(this, (tochan) ? this->name : nick, tochan, nick, userhost, restptr);
		return;
	case OER_WHICHCOMMAND_LOGON:
#ifdef OER_DEBUG
		oer_debug(OER_DEBUG_INFO, "parsecommand->LOGON\n");
#endif
		if(oerperl_call_commandhandlers(command, restptr, target, nick, userhost, 1) == OERPERL_BLOCK_OER_COMMAND) {
			return;
		}
                /* channel is optional */
                this = getchptr(channel);
		logoncommand(this, (tochan) ? this->name : nick, tochan, nick, userhost, restptr);
		return;
	case OER_WHICHCOMMAND_INFO:
#ifdef OER_DEBUG
		oer_debug(OER_DEBUG_INFO, "parsecommand->INFO\n");
#endif
		if(oerperl_call_commandhandlers(command, restptr, target, nick, userhost, 1) == OERPERL_BLOCK_OER_COMMAND) {
			return;
		}
		/* channel is optional */
                this = getchptr(channel);
                if(!checkauth(this, userhost, nick, tochan, 1)) {
                        return;
                }
		snprintf(outstring, WRITE_BUFFER_LENGTH, "%s, %s -- oer+MySQL was written in August of 2000. This version of oer+MySQL was compiled %s %s. The C source of this oer+MySQL version consists of %d lines totalling %d bytes.", OER_VERSION, OER_COPYRIGHT1, __DATE__, __TIME__, SOURCE_LINES, SOURCE_CHARS);
		sendreply((tochan) ? channel : nick, tochan, 0, OER_TIMED_PRIORITY_NORMAL, outstring);
		return;
	case OER_WHICHCOMMAND_UPTIME:
#ifdef OER_DEBUG
		oer_debug(OER_DEBUG_INFO, "parsecommand->UPTIME\n");
#endif
		if(oerperl_call_commandhandlers(command, restptr, target, nick, userhost, 1) == OERPERL_BLOCK_OER_COMMAND) {
			return;
		}
		/* channel is optional */
                this = getchptr(channel);
                if(!checkauth(this, userhost, nick, tochan, 1)) {
                        return;
                }
		secondstostring(mystate->now - mystate->startup, uptimestring, STRINGLEN);
		snprintf(outstring, WRITE_BUFFER_LENGTH, "oer+MySQL uptime: %s", uptimestring);
		if(uptime(&uptime_seconds, &idle_seconds)) {
			secondstostring(uptime_seconds, uptimestring, STRINGLEN);
			idle_per = idle_seconds % uptime_seconds;
                        idle_per = uptime_seconds - idle_per;
                        idle_per *= 100;
                        idle_per /= uptime_seconds;
                        idle_per = 100 - idle_per;
#ifdef OER_UPTIME_SHOW_IDLE
			snprintf(outstring2, WRITE_BUFFER_LENGTH, ", host uptime: %s (host idle: %lu%%)", uptimestring, idle_per);
#else
			snprintf(outstring2, WRITE_BUFFER_LENGTH, ", host uptime: %s", uptimestring);
#endif
			strncat(outstring, outstring2, WRITE_BUFFER_LENGTH - strlen(outstring));
		}
		sendreply((tochan) ? channel : nick, tochan, 0, OER_TIMED_PRIORITY_NORMAL, outstring);
		return;
	case OER_WHICHCOMMAND_ACTION:
#ifdef OER_DEBUG
		oer_debug(OER_DEBUG_INFO, "parsecommand->ACTION\n");
#endif
		if(oerperl_call_commandhandlers(command, restptr, target, nick, userhost, 1) == OERPERL_BLOCK_OER_COMMAND) {
			return;
		}
		if((this = getchptr(channel)) == NULL) {
                        return;
                }
                if(!isadmin(this, userhost)) {
                        if(haschanflags(this, "X")) {
                                return;
                        }
                        if(!checkauth(this, userhost, nick, 1, 0)) {
                                return;
                        }
                }
		if(strlen(restptr) > 0) {
			snprintf(outstring, WRITE_BUFFER_LENGTH, "%cACTION %s%c", 1, restptr, 1);
			sendreply(channel, 1, 0, OER_TIMED_PRIORITY_NORMAL, outstring);
		}
		return;
	case OER_WHICHCOMMAND_SAY:
#ifdef OER_DEBUG
		oer_debug(OER_DEBUG_INFO, "parsecommand->SAY\n");
#endif
		if(oerperl_call_commandhandlers(command, restptr, target, nick, userhost, 1) == OERPERL_BLOCK_OER_COMMAND) {
			return;
		}
		if((this = getchptr(channel)) == NULL) {
                        return;
                }
                if(!isadmin(this, userhost)) {
                        if(haschanflags(this, "X")) {
                                return;
                        }
                        if(!checkauth(this, userhost, nick, 1, 0)) {
                                return;
                        }
                }
		if(strlen(restptr) > 0) {
			sendreply(channel, 1, 0, OER_TIMED_PRIORITY_NORMAL, restptr);
		}
		return;
	case OER_WHICHCOMMAND_RAW:
#ifdef OER_DEBUG
		oer_debug(OER_DEBUG_INFO, "parsecommand->RAW\n");
#endif
		if(oerperl_call_commandhandlers(command, restptr, target, nick, userhost, 1) == OERPERL_BLOCK_OER_COMMAND) {
			return;
		}
		if(!isopa(userhost)) {
			return;
		}
		restptr = commandline + prevppos;
		if(!strlen(restptr)) {
			return;
		}
		snprintf(timed_str, WRITE_BUFFER_LENGTH, "%s", restptr);
		timed_new(NULL, mystate->now, OER_TIMED_TYPE_NORMAL, OER_TIMED_PRIORITY_CHANNEL_PROTECTION, timed_str);
		return;
	case OER_WHICHCOMMAND_LAST:
#ifdef OER_DEBUG
		oer_debug(OER_DEBUG_INFO, "parsecommand->LAST\n");
#endif
		if(oerperl_call_commandhandlers(command, restptr, target, nick, userhost, 1) == OERPERL_BLOCK_OER_COMMAND) {
			return;
		}
		if((this = getchptr(channel)) == NULL) {
                        return;
                }
                if(!checkauth(this, userhost, nick, 1, 1)) {
                        return;
                }
                showlast(this, (tochan) ? this->name : nick, tochan, restptr);
                return;
	case OER_WHICHCOMMAND_QUOTE:
#ifdef OER_DEBUG
		oer_debug(OER_DEBUG_INFO, "parsecommand->QUOTE\n");
#endif
		if(oerperl_call_commandhandlers(command, restptr, target, nick, userhost, 1) == OERPERL_BLOCK_OER_COMMAND) {
			return;
		}
                if((this = getchptr(channel)) == NULL) {
                        return;
                }
                if(!checkauth(this, userhost, nick, 1, 1)) {
                        return;
                }
		if(getrandommsg(this, restptr, NICKLEN, outstring, WRITE_BUFFER_LENGTH, 1) < 0) {
			return;
		}
		snprintf(outstring2, WRITE_BUFFER_LENGTH, "\"%s\" (c) %s", outstring, restptr);
		sendreply((tochan) ? channel : nick, tochan, 0, OER_TIMED_PRIORITY_NORMAL, outstring2);
		return;
	case OER_WHICHCOMMAND_SEEN:
#ifdef OER_DEBUG
		oer_debug(OER_DEBUG_INFO, "parsecommand->SEEN\n");
#endif
		if(oerperl_call_commandhandlers(command, restptr, target, nick, userhost, 1) == OERPERL_BLOCK_OER_COMMAND) {
			return;
		}
		if((this = getchptr(channel)) == NULL) {
                        return;
                }
                if(!checkauth(this, userhost, nick, 1, 1)) {
                        return;
                }
                showseen(this, (tochan) ? channel : nick, tochan, restptr);
		return;
	case OER_WHICHCOMMAND_NSTATS:
#ifdef OER_DEBUG
		oer_debug(OER_DEBUG_INFO, "parsecommand->NSTATS\n");
#endif
		if(oerperl_call_commandhandlers(command, restptr, target, nick, userhost, 1) == OERPERL_BLOCK_OER_COMMAND) {
			return;
		}
                if((this = getchptr(channel)) == NULL) {
                        return;
                }
                if(!checkauth(this, userhost, nick, 1, 1)) {
                        return;
                }
                nstats(this, (tochan) ? channel : nick, tochan, restptr);
		return;
	case OER_WHICHCOMMAND_WALL:
#ifdef OER_DEBUG
		oer_debug(OER_DEBUG_INFO, "parsecommand->WALL\n");
#endif
		if(oerperl_call_commandhandlers(command, restptr, target, nick, userhost, 1) == OERPERL_BLOCK_OER_COMMAND) {
			return;
		}
                if(tochan) {
                        /* makes no sense */
                        return;
                }
                if((this = getchptr(channel)) == NULL) {
                        return;
                }
                if(!checkauth(this, userhost, nick, 1, 0)) {
                        return;
                }
		wall(this, nick, userhost, restptr);
		return;
	case OER_WHICHCOMMAND_CLONECHANNEL:
#ifdef OER_DEBUG
                oer_debug(OER_DEBUG_INFO, "parsecommand->CLONECHANNEL\n");
#endif
		if(oerperl_call_commandhandlers(command, restptr, target, nick, userhost, 1) == OERPERL_BLOCK_OER_COMMAND) {
			return;
		}
                if(!isopa(userhost)) {
                        return;
                }
                if((that = clonechannel(channel, restptr)) == NULL) {
                        return;
                }
                snprintf(outstring, WRITE_BUFFER_LENGTH, "cloned channel %s to %s", channel, that->name);
                sendreply((tochan) ? channel : nick, tochan, 0, OER_TIMED_PRIORITY_NORMAL, outstring);
		if(index(mystate->flags, (int)'n') == NULL) {
			/* old type of last/seen */
			if(!haschanflags(that, "L") && !haschanflags(that, "S")) {
				return;
			}
			mysqldbname(that->name, outstring2, WRITE_BUFFER_LENGTH);
			snprintf(outstring, WRITE_BUFFER_LENGTH, "database tables last_%s and seen_%s have to be created manually using the provided oer+MySQL-last.sql and oer+MySQL-seen.sql scripts", outstring2, outstring2);
			sendreply((tochan) ? channel : nick, tochan, 0, OER_TIMED_PRIORITY_NORMAL, outstring);
		}
                return;
	case OER_WHICHCOMMAND_SSTATS:
#ifdef OER_DEBUG
		oer_debug(OER_DEBUG_INFO, "parsecommand->SSTATS\n");
#endif
		if(oerperl_call_commandhandlers(command, restptr, target, nick, userhost, 1) == OERPERL_BLOCK_OER_COMMAND) {
			return;
		}
		/* channel is optional */
                this = getchptr(channel);
                if(!checkauth(this, userhost, nick, tochan, 1)) {
                        return;
                }
		secondstostring(mystate->now - mystate->current_server->linkup, uptimestring, STRINGLEN);
		snprintf(outstring, WRITE_BUFFER_LENGTH, "%s RX/TX bytes: %d/%d, link connected for %s", (strlen(mystate->current_server->serverhost_r) > 0) ? mystate->current_server->serverhost_r : mystate->current_server->serverhost, mystate->current_server->rx, mystate->current_server->tx, uptimestring);
		sendreply((tochan) ? channel : nick, tochan, 0, OER_TIMED_PRIORITY_NORMAL, outstring);
		return;
	case OER_WHICHCOMMAND_DATE:
#ifdef OER_DEBUG
                oer_debug(OER_DEBUG_INFO, "parsecommand->DATE\n");
#endif
		if(oerperl_call_commandhandlers(command, restptr, target, nick, userhost, 1) == OERPERL_BLOCK_OER_COMMAND) {
			return;
		}
		/* channel is optional */
                this = getchptr(channel);
                if(!checkauth(this, userhost, nick, tochan, 1)) {
                        return;
                }
                strncpy(ts, ctime(&mystate->now), TINYSTRINGLEN);
                striplf(ts);
                tzset();
                snprintf(outstring, WRITE_BUFFER_LENGTH, "my date is %s %s %s", ts, tzname[0], tzname[1]);
                sendreply((tochan) ? channel : nick, tochan, 0, OER_TIMED_PRIORITY_NORMAL, outstring);
                return;
	case OER_WHICHCOMMAND_USERCOPY:
#ifdef OER_DEBUG
                oer_debug(OER_DEBUG_INFO, "parsecommand->USERCOPY\n");
#endif
		if(oerperl_call_commandhandlers(command, restptr, target, nick, userhost, 1) == OERPERL_BLOCK_OER_COMMAND) {
			return;
		}
                if((this = getchptr(channel)) == NULL) {
                        return;
                }
                if(!isadmin(this, userhost)) {
                        return;
                }
                ppos = 0;
                if((ppos = parse(restptr, ppos, " ", p1, STRINGLEN, 0)) < 0) {
                        return;
                }
                if((ppos = parse(restptr, ppos, " ", p2, STRINGLEN, 0)) < 0) {
                        return;
                }
                if((ppos = parse(restptr, ppos, " ", p3, STRINGLEN, 0)) < 0) {
                        return;
                }
                if((that = getchptr(p2)) == NULL) {
                        return;
                }
                ptr = (strlen(p3)) ? p3 : NULL;
                if((user = usercopy(this, p1, that, ptr)) == NULL) {
                        return;
                }
		if(ptr == NULL) {
                        snprintf(outstring, WRITE_BUFFER_LENGTH, "%s user %s copied without options to %s", this->name, user->handle, that->name);
                } else {
                        snprintf(outstring, WRITE_BUFFER_LENGTH, "%s user %s copied without options to %s as %s", this->name, user->handle, that->name, ptr);
                }
                sendreply((tochan) ? channel : nick, tochan, 0, OER_TIMED_PRIORITY_NORMAL, outstring);
                return;
	case OER_WHICHCOMMAND_CLEARBANS:
#ifdef OER_DEBUG
                oer_debug(OER_DEBUG_INFO, "parsecommand->CLEARBANS\n");
#endif
		if(oerperl_call_commandhandlers(command, restptr, target, nick, userhost, 1) == OERPERL_BLOCK_OER_COMMAND) {
			return;
		}
		if((this = getchptr(channel)) == NULL) {
                        return;
                }
                if(!this->i_am_op) {
                        return;
                }
                if(!checkauth(this, userhost, nick, 1, 0)) {
                        return;
                }
                for(cb = this->channelbans; cb != NULL; cb = cb->next) {
			if(ispermban(this, cb->ban)) {
                                continue;
                        }
                        mmode_new(this, mystate->now, "-b", cb->ban);
                        delchannelban(this, cb->ban);
                }
                return;
	case OER_WHICHCOMMAND_CYCLE:
#ifdef OER_DEBUG
                oer_debug(OER_DEBUG_INFO, "parsecommand->CYCLE\n");
#endif
		if(oerperl_call_commandhandlers(command, restptr, target, nick, userhost, 1) == OERPERL_BLOCK_OER_COMMAND) {
			return;
		}
                if((this = getchptr(channel)) == NULL) {
                        return;
                }
                if(!isadmin(this, userhost)) {
                        return;
                }
#ifdef OER_DEBUG
                oer_debug(OER_DEBUG_INFO, "parsecommand)->cycling %s because %s!%s ordered me to\n", this->name, nick, userhost);
#endif
                snprintf(timed_str, WRITE_BUFFER_LENGTH, "PART %s", this->name);
                timed_new(NULL, mystate->now, OER_TIMED_TYPE_NORMAL, OER_TIMED_PRIORITY_CHANNEL_HANDLING, timed_str);
                return;
	case OER_WHICHCOMMAND_BSTATS:
#ifdef OER_DEBUG
                oer_debug(OER_DEBUG_INFO, "parsecommand->BSTATS\n");
#endif
		if(oerperl_call_commandhandlers(command, restptr, target, nick, userhost, 1) == OERPERL_BLOCK_OER_COMMAND) {
			return;
		}
		/* channel is optional */
                this = getchptr(channel);
                if(!checkauth(this, userhost, nick, tochan, 1)) {
                        return;
                }
                numofchans = 0;
                numofadmins = 0;
                numofusers = 0;
                numofchanusers = 0;
                numofadminmasks = 0;
                numofusermasks = 0;
                for(admin = mystate->admins; admin != NULL; admin = admin->next) {
                        numofadmins++;
                        for(ms = admin->firstmask; ms != NULL; ms = ms->next) {
                                numofadminmasks++;
                        }
                }
		for(that = mystate->channels; that != NULL; that = that->next) {
			/* first gather statistics */
                        numofchans++;
                        for(u = that->nicks; u != NULL; u = u->next) {
                                numofchanusers++;
                        }
                        for(user = that->users; user != NULL; user = user->next) {
                                numofusers++;
                                for(ms = user->firstmask; ms != NULL; ms = ms->next) {
                                        numofusermasks++;
                                }
                        }
                }
		snprintf(outstring, WRITE_BUFFER_LENGTH, "%d %s (%d %s), %d %s (%d %s), %d channel %s and %d %s", numofadmins, (numofadmins == 1) ? "admin" : "admins", numofadminmasks, (numofadminmasks == 1) ? "mask" : "masks", numofusers, (numofusers == 1) ? "user" : "users", numofusermasks, (numofusermasks == 1) ? "mask" : "masks", numofchanusers, (numofchanusers == 1) ? "user" : "users", numofchans, (numofchans == 1) ? "channel" : "channels");
                sendreply((tochan) ? channel : nick, tochan, 0, OER_TIMED_PRIORITY_NORMAL, outstring);
                return;
	case OER_WHICHCOMMAND_EXT:
#ifdef OER_DEBUG
		oer_debug(OER_DEBUG_INFO, "parsecommand->EXT\n");
#endif
		if(oerperl_call_commandhandlers(command, restptr, target, nick, userhost, 1) == OERPERL_BLOCK_OER_COMMAND) {
			return;
		}
                /* channel is optional */
                this = getchptr(channel);
                if(!checkauth(this, userhost, nick, tochan, 1)) {
                        return;
                }
		extcommand(this, (tochan) ? this->name : nick, tochan, nick, userhost, restptr);
		return;
	case OER_WHICHCOMMAND_DBCLOSE:
#ifdef OER_DEBUG
		oer_debug(OER_DEBUG_INFO, "parsecommand->DBCLOSE\n");
#endif
		if(oerperl_call_commandhandlers(command, restptr, target, nick, userhost, 1) == OERPERL_BLOCK_OER_COMMAND) {
			return;
		}
		if(!isopa(userhost)) {
			return;
		}
		dbclose();
		snprintf(outstring, WRITE_BUFFER_LENGTH, "connection to database(s) closed");
                sendreply((tochan) ? channel : nick, tochan, 0, OER_TIMED_PRIORITY_NORMAL, outstring);
		return;
	case OER_WHICHCOMMAND_DBCONNECT:
#ifdef OER_DEBUG
		oer_debug(OER_DEBUG_INFO, "parsecommand->DBCONNECT\n");
#endif
		if(oerperl_call_commandhandlers(command, restptr, target, nick, userhost, 1) == OERPERL_BLOCK_OER_COMMAND) {
			return;
		}
		if(!isopa(userhost)) {
			return;
		}
		if(dbpingall()) {
			snprintf(outstring, WRITE_BUFFER_LENGTH, "(re)connected to database(s)");
			mystate->mysqldb->isdead = 0;
			mystate->mysqldb->isalive = mystate->now;
			mystate->mysqldb->tod = 0;
			if(mystate->mysqladmins != mystate->mysqldb) {
				mystate->mysqladmins->isdead = 0;
				mystate->mysqladmins->isalive = mystate->now;
				mystate->mysqladmins->tod = 0;
			}
			if(mystate->mysqlusers != mystate->mysqldb) {
				mystate->mysqlusers->isdead = 0;
				mystate->mysqlusers->isalive = mystate->now;
				mystate->mysqlusers->tod = 0;
			}
		} else {
			snprintf(outstring, WRITE_BUFFER_LENGTH, "couldn't reconnect to database(s)");
		}
		sendreply((tochan) ? channel : nick, tochan, 0, OER_TIMED_PRIORITY_NORMAL, outstring);
		return;
	case OER_WHICHCOMMAND_RESET:
#ifdef OER_DEBUG
                oer_debug(OER_DEBUG_INFO, "parsecommand->RESET\n");
#endif
		if(oerperl_call_commandhandlers(command, restptr, target, nick, userhost, 1) == OERPERL_BLOCK_OER_COMMAND) {
			return;
		}
                if((this = getchptr(channel)) == NULL) {
                        return;
                }
                if(!checkauth(this, userhost, nick, 1, 0)) {
                        return;
                }
                resetcommand(this, (tochan) ? this->name : nick, tochan, nick, userhost, restptr);
                return;
	case OER_WHICHCOMMAND_CHANINFO:
#ifdef OER_DEBUG
                oer_debug(OER_DEBUG_INFO, "parsecommand->CHANINFO\n");
#endif
		if(oerperl_call_commandhandlers(command, restptr, target, nick, userhost, 1) == OERPERL_BLOCK_OER_COMMAND) {
			return;
		}
		if((this = getchptr(channel)) == NULL) {
                        return;
                }
                if(!checkauth(this, userhost, nick, 1, 1)) {
                        return;
                }
		if(!this->joined) {
                        snprintf(outstring, WRITE_BUFFER_LENGTH, "I have yet to join %s", this->name);
                        sendreply((tochan) ? channel : nick, tochan, 0, OER_TIMED_PRIORITY_NORMAL, outstring);
                        return;
                }
                if(this->joining) {
                        snprintf(outstring, WRITE_BUFFER_LENGTH, "I haven't yet finished joining %s", this->name);
                        sendreply((tochan) ? channel : nick, tochan, 0, OER_TIMED_PRIORITY_NORMAL, outstring);
                        return;
                }
                numofmmodes = getmmodecount(this);
		for(u = this->nicks, numofoppedchanusers = 0, numofvoicedchanusers = 0, numofopers = 0, i_am_voiced = 0; u != NULL; u = u->next) {
                        if(u->chanop) {
                                numofoppedchanusers++;
                        }
                        if(u->voice) {
                                numofvoicedchanusers++;
                        }
                        if(u->ircop) {
                                numofopers++;
                        }
                        if(!strcasecmp(u->nick, mystate->nick) && u->voice) {
                                i_am_voiced = 1;
                        }
                }
                snprintf(outstring, WRITE_BUFFER_LENGTH, "%s has %d %s: %d opped%s, %d voiced%s, %d IRC %s and %d scheduled %s", this->name, this->nickcount, (this->nickcount == 1) ? "user" : "users", numofoppedchanusers, (this->i_am_op == 1) ? "(@)" : "", numofvoicedchanusers, (i_am_voiced == 1) ? "(+)" : "", numofopers, (numofopers == 1) ? "operator" : "operators", numofmmodes, (numofmmodes == 1) ? "mmode" : "mmodes");
                sendreply((tochan) ? channel : nick, tochan, 0, OER_TIMED_PRIORITY_NORMAL, outstring);
                return;
	case OER_WHICHCOMMAND_QUEUESTATS:
#ifdef OER_DEBUG
                oer_debug(OER_DEBUG_INFO, "parsecommand->QUEUESTATS\n");
#endif
		if(oerperl_call_commandhandlers(command, restptr, target, nick, userhost, 1) == OERPERL_BLOCK_OER_COMMAND) {
			return;
		}
                /* channel is optional */
                this = getchptr(channel);
                if(!checkauth(this, userhost, nick, tochan, 1)) {
                        return;
                }
                for(numofmmodes = 0, that = mystate->channels; that != NULL; that = that->next) {
                        numofmmodes += getmmodecount(that);
                }
                numoftimeds = gettimedcount();
                snprintf(outstring, WRITE_BUFFER_LENGTH, "there are %d scheduled %s and %d scheduled %s (all channels)", numoftimeds, (numoftimeds == 1) ? "timed" : "timeds", numofmmodes, (numofmmodes == 1) ? "mmode" : "mmodes");
                sendreply((tochan) ? channel : nick, tochan, 0, OER_TIMED_PRIORITY_NORMAL, outstring);
                return;
        case OER_WHICHCOMMAND_INVALID:
			if(oerperl_call_commandhandlers(command, restptr, target, nick, userhost, 0)) {
				return;
			}
#ifdef OER_DEBUG
		oer_debug(OER_DEBUG_INFO, "parsecommand->invalid or malformed command %s\n", command);
#endif
		break;
	}
}

int checkauth(struct channel *this, char *userhost, char *nick, int tochan, int is_s)
{
        int proceed;
        struct channel *that;
#ifdef OER_DEBUG
        oer_debug(OER_DEBUG_INFO, "checkauth(\"%s\", \"%s\", \"%s\", %d, %d)\n", this->name, userhost, nick, tochan, is_s);
#endif
        if((proceed = (isopa(userhost)))) {
                return 1;
        }
	/* if we get here proceed = 0 */
        if(tochan) {
                if(haschanflags(this, "U")) {
                        /* we require at least "o" */
                        if(isatleastop(this, nick, userhost)) {
                                proceed = 1;
                        }
                } else if(isatleastopnow(this, nick, userhost)) {
                        /* @ is sufficient */
                        proceed = 1;
                }
                if(is_s && haschanflags(this, "s")) {
                        proceed = 1;
                }
        } else {
                for(that = mystate->channels; that != NULL && !proceed; that = that->next) {
                        if(haschanflags(that, "U")) {
                                /* we require at least "o" */
                                if(isatleastop(that, nick, userhost)) {
                                        proceed = 1;
                                }
                        } else if(isatleastopnow(that, nick, userhost)) {
                                /* @ is sufficient */
                                proceed = 1;
                        }
                }
        }
        return proceed;
}

void parsenotice(char *nick, char *userhost, char *target, char *message)
{
	int ppos;
	char command[STRINGLEN + 1];
	char p1[STRINGLEN + 1];
	struct channel *this;
#ifdef OER_DEBUG
        oer_debug(OER_DEBUG_NOISE, "parsenotice(\"%s\", \"%s\", \"%s\", \"%s\")\n", nick, userhost, target, message);
#endif
        if((this = getchptr(target)) != NULL) {
		/* check for notice flood */
		if(this->i_am_op && checkforansi(this, nick, userhost, message)) {
			return;
		}
		if(this->i_am_op && checkforflood(this, nick, userhost, message)) {
			return;
		}
		if(this->i_am_op && checkforbadword(this, nick, userhost, message)) {
			return;
		}
		/* only add this consequetive line iff the user
		   isn't being kicked already (lag between kick and last message) */
		if(!gettobek(this, nick)) {
			addnewpubmsg(this, mystate->now, nick, userhost, message);
		}
		return;
	}
	/* check for sync message only if destined to us */
	ppos = 0;
	if((ppos = parse(message, ppos, " ", command, STRINGLEN, 0)) < 0) {
		return;
	}
	if(!strcasecmp(command, "oer_internal_sync_message")) {
		/* admin or user sync request, sent by other oer+MySQL
		   instances in a shared oer+MySQL setup */
		for(this = mystate->channels; this != NULL; this = this->next) {
			/* check for user flag "s" on any of our channels */
			if(hasuserflags(this, userhost, "s", "")) {
				break;
			}
		}
		if(this == NULL) {
			return;
		}
		if((ppos = parse(message, ppos, " ", p1, STRINGLEN, 0)) < 0) {
			return;
		}
		if(isvalidchannel(p1) && ((this = getchptr(p1)) != NULL)) {
#ifdef OER_DEBUG
			oer_debug(OER_DEBUG_INFO, "parsenotice->received sync request for %s from %s!%s\n", this->name, nick, userhost);
#endif
			/* sync channel */
			syncusers(this);
			mystate->last_ts_check = mystate->now;
		} else if(!strcasecmp(p1, "admins")) {
#ifdef OER_DEBUG
			oer_debug(OER_DEBUG_INFO, "parsenotice->received admin sync request from %s\n", userhost);
#endif
			/* sync admins */
			syncadmins();
			mystate->last_ts_check = mystate->now;
		}
	}
}

