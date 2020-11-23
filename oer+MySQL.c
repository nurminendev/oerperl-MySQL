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
#include "ds.h"
#include "network.h"
#include "misc.h"
#include "perl.h"

/* global (to all) variables */

int do_console;
int do_debug;
int do_tempoutput;

char salt_chars[] = { "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz" };

/* the following are static buffers used throughout the code, saves
   CPU cycles and stack space */
char timed_str[WRITE_BUFFER_LENGTH + 1];
char mysql_safe_str1[(WRITE_BUFFER_LENGTH * 2) + 1];
char mysql_safe_str2[(WRITE_BUFFER_LENGTH * 2) + 1];
char mysql_safe_str3[(WRITE_BUFFER_LENGTH * 2) + 1];
char mysql_safe_str4[(WRITE_BUFFER_LENGTH * 2) + 1];

struct state *mystate;
PerlInterpreter *my_perl;

/* global (to oer.c) variables */
int do_signals;

/* prototype definitions */
static void handlesignal(int);

int main(int argc, char *argv[])
{
	extern char *optarg;
        extern int optind;
        extern int opterr;
        extern int optopt;
#ifdef HAVE_LOCALE_H
        char *locptr;
#endif
	char config[STRINGLEN + 1];
        char adminsfrom[IDENTLEN + 1];
        char usersfrom[IDENTLEN + 1];
        char state[STRINGLEN + 1];
	int retval;
	int c;
	int do_help;
	int do_version;
	int do_adminsfrom;
	int do_usersfrom;
	int do_state;
	fd_set rsock;
	pid_t newpid;
	struct timeval tv;
#ifdef NEW_SIGNALS
        struct sigaction act;
        struct sigaction old;
#endif
	/* set initial settings that affect ex. oer_debug() */
	do_console = 0;
	do_debug = OER_DEBUG_INFO;
	do_help = 0;
	do_signals = 1;
	do_version = 0;
	do_tempoutput = 1;
	do_adminsfrom = 0;
	do_usersfrom = 0;
	do_state = 0;
	strncpy(config, "oer+MySQL.conf", STRINGLEN);
        while((c = getopt(argc, argv, "a:cd:f:hsu:t:v")) > 0) {
		switch(c) {
		case 'a':
			strncpy(adminsfrom, optarg, IDENTLEN);
                        oer_debug(OER_DEBUG_INFO, "main->I will get my admins with ident %s\n", adminsfrom);
                        do_adminsfrom = 1;
                        break;
		case 'c':
			do_console = 1;
			break;
		case 'd':
			do_debug = atoi(optarg);
			break;
		case 'f':
			strncpy(config, optarg, STRINGLEN);
			break;
		case 'h':
			do_help = 1;
			break;
		case 's':
			do_signals = 0;
			break;
		case 'u':
			strncpy(usersfrom, optarg, IDENTLEN);
                        oer_debug(OER_DEBUG_INFO, "main->I will get my users with ident %s\n", usersfrom);
                        do_usersfrom = 1;
                        break;
		case 't':
			strncpy(state, optarg, STRINGLEN);
			oer_debug(OER_DEBUG_INFO, "main->my state is %s\n", state);
			do_state = 1;
			break;
		case 'v':
			do_version = 1;
			break;
		default:
			exit(EXIT_FAILURE);
		}
	}
	if(do_help) {
		oer_debug(OER_DEBUG_INFO, "usage: %s [-a <ident>] [-c] [-d <level>] [-f <config>] [-h] [-s] [-u <ident>] [-t <state>] [-v]\n", argv[0]);
		exit(EXIT_SUCCESS);
	}
	oer_debug(OER_DEBUG_INFO, "%s, %s\n\n", OER_VERSION, OER_COPYRIGHT1);
	oer_debug(OER_DEBUG_INFO, "%s\n", OER_COPYRIGHT2);
	if(do_version) {
		exit(EXIT_SUCCESS);
	}
#ifdef HAVE_LOCALE_H
        if((locptr = setlocale(LC_CTYPE, "")) == NULL) {
                oer_debug(OER_DEBUG_FATAL, "main->setlocale() failed\n");
                exit(EXIT_FAILURE);
        }
#endif
#ifdef NEW_SIGNALS
        sigemptyset(&act.sa_mask);
        act.sa_flags = SA_RESTART;
        act.sa_handler = handlesignal;
        if(sigaction(SIGINT, &act, &old) < 0) {
                oer_debug(OER_DEBUG_FATAL, "main->failed to set SIGINT: %s\n", strerror(errno));
                exit(EXIT_FAILURE);
        }
        if(sigaction(SIGTERM, &act, &old) < 0) {
                oer_debug(OER_DEBUG_FATAL, "main->failed to set SIGTERM: %s\n", strerror(errno));
                exit(EXIT_FAILURE);
        }
        if(sigaction(SIGPIPE, &act, &old) < 0) {
                oer_debug(OER_DEBUG_FATAL, "main->failed to set SIGPIPE: %s\n", strerror(errno));
                exit(EXIT_FAILURE);
        }
        if(sigaction(SIGUSR1, &act, &old) < 0) {
                oer_debug(OER_DEBUG_FATAL, "main->failed to set SIGUSR1: %s\n", strerror(errno));
                exit(EXIT_FAILURE);
        }
#else
	if(signal(SIGINT, handlesignal) == SIG_ERR) {
                oer_debug(OER_DEBUG_FATAL, "main->failed to set SIGINT: %s\n", strerror(errno));
                exit(EXIT_FAILURE);
        }
	if(signal(SIGTERM, handlesignal) == SIG_ERR) {
                oer_debug(OER_DEBUG_FATAL, "main->failed to set SIGTERM: %s\n", strerror(errno));
                exit(EXIT_FAILURE);
        }
	if(signal(SIGPIPE, handlesignal) == SIG_ERR) {
                oer_debug(OER_DEBUG_FATAL, "main->failed to set SIGPIPE: %s\n", strerror(errno));
                exit(EXIT_FAILURE);
        }
	if(signal(SIGUSR1, handlesignal) == SIG_ERR) {
                oer_debug(OER_DEBUG_FATAL, "main->failed to set SIGUSR1: %s\n", strerror(errno));
                exit(EXIT_FAILURE);
        }
#endif
	if((mystate = emptystate()) == NULL) {
		oer_debug(OER_DEBUG_FATAL, "main->failed to allocate memory for oer+MySQL main state\n");
		exit(EXIT_FAILURE);
	}
	if(!oerperl_init()) {
		oer_debug(OER_DEBUG_FATAL, "main->failed to initialize perl interpreter\n");
		exit(EXIT_FAILURE);
	}
	atexit(oerperl_shutdown);
	strncpy(mystate->config, config, STRINGLEN);
	if(do_state) {
		strcpy(mystate->state, state);
	}
	if(do_adminsfrom) {
                strcpy(mystate->adminsfrom, adminsfrom);
        }
        if(do_usersfrom) {
                strcpy(mystate->usersfrom, usersfrom);
        }
	if(!loadconf(mystate->config)) {
                oer_debug(OER_DEBUG_FATAL, "main->loadconf() failed\n");
		exit(EXIT_FAILURE);
        }
	/* free mystate->mysqladmins if not needed */
	if(isinitialmysql(mystate->mysqladmins) || issamemysql(mystate->mysqladmins, mystate->mysqldb)) {
		free(mystate->mysqladmins);
		mystate->mysqladmins = mystate->mysqldb;
	}
	/* free mystate->mysqlusers if not needed */
	if(isinitialmysql(mystate->mysqlusers) || issamemysql(mystate->mysqlusers, mystate->mysqldb)) {
		free(mystate->mysqlusers);
		mystate->mysqlusers = mystate->mysqldb;
	}
	if(!dbconnect(mystate->mysqldb)) {
		exit(EXIT_FAILURE);
        }
	mystate->mysqldb->isalive = time(NULL);
	if(mystate->mysqladmins != mystate->mysqldb) {
		if(!dbconnect(mystate->mysqladmins)) {
			exit(EXIT_FAILURE);
		}
		mystate->mysqladmins->isalive = time(NULL);
	}
	if(mystate->mysqlusers != mystate->mysqldb) {
		if(!dbconnect(mystate->mysqlusers)) {
			exit(EXIT_FAILURE);
		}
		mystate->mysqlusers->isalive = time(NULL);
	}
        if(do_console) {
		oer_debug(OER_DEBUG_INFO, "console mode is on (debug messages will be shown on screen)\n");
	}
	/* switch temp output off for initenv() */
	do_tempoutput = 0;
	if(!initenv()) {
                oer_debug(OER_DEBUG_FATAL, "main->initenv() failed\n");
		exit(EXIT_FAILURE);
        }
	/* on again */
	do_tempoutput = 1;
	if(index(mystate->flags, (int)'o') != NULL) {
		oer_debug(OER_DEBUG_INFO, "debug messages will be saved to MySQL\n");
	}
	if(index(mystate->flags, (int)'l') != NULL) {
		oer_debug(OER_DEBUG_INFO, "IRC traffic will be logged to MySQL\n");
	}
	if(!do_console) {
		oer_debug(OER_DEBUG_INFO, "main->oer+MySQL calling fork()\n");
		newpid = fork();
		if(newpid < 0) {
			oer_debug(OER_DEBUG_FATAL, "main->couldn't fork()\n");
			exit(EXIT_FAILURE);
		}
		if(newpid) {
			/* the parent */
			oer_debug(OER_DEBUG_INFO, "main->fork successful, oer+MySQL new pid is %ld (parent exiting)\n", newpid);
			exit(EXIT_SUCCESS);
		}
		/* necessary for some remote progs */
		close(STDIN_FILENO);
		close(STDOUT_FILENO);
		close(STDERR_FILENO);
		do_console = 0;
	}
	/* from now on we will write to console/database */
	do_tempoutput = 0;
	establishconnection();
	if(mystate->bailout) {
                exit(EXIT_FAILURE);
        }
	mystate->loopforever = 1;
	while(mystate->loopforever) {
#ifdef OER_DEBUG
		oer_debug(OER_DEBUG_FLOOD, "main->mainloop\n");
#endif
		FD_ZERO(&rsock);
		FD_SET(mystate->sockfd, &rsock);
		tv.tv_sec = OER_MAINLOOP_TIMEOUT_SECS;
		tv.tv_usec = OER_MAINLOOP_TIMEOUT_USECS;
		retval = select(mystate->sockfd + 1, &rsock, NULL, NULL, &tv);
                mystate->now = time(NULL);
		if(retval < 0) {
			if(errno != EINTR) {
				oer_debug(OER_DEBUG_ERROR, "main->select() failed: %s\n", strerror(errno));
				mystate->reconnect = OER_RECONNECT_ERROR;
				reconnect();
			}
			continue;
		}
		if(retval > 0) {
			/* we have server I/O */
			if(FD_ISSET(mystate->sockfd, &rsock)) {
				switch(handleserverdata()) {
				case OER_HANDLESERVERDATA_ERR_WRITE:
				case OER_HANDLESERVERDATA_ERR_READ:
				case OER_HANDLESERVERDATA_ERR_NOTHING_READ:
					mystate->reconnect = OER_RECONNECT_ERROR;
					reconnect();
					break;
				}
			}
		}
		reconnect();
		/* retval == 0, timeout */
		switch(registerconnection()) {
		case OER_REGISTERCONNECTION_ERR_GETPWUID:
			exit(EXIT_FAILURE);
		case OER_REGISTERCONNECTION_ERR_WRITE:
		case OER_REGISTERCONNECTION_ERR_HANDLESERVERDATA:
			mystate->reconnect = OER_RECONNECT_ERROR;
			reconnect();
			break;
		case OER_REGISTERCONNECTION_ALREADY_REGISTERED:
		case OER_REGISTERCONNECTION_EVERYTHING_OK:
			break;
		}
		/* do some housekeeping */
                processenv();
                /* convert all mmodes to timeds */
                mmodes2timeds();
                /* process timeds */
                switch(processtimeds()) {
                case OER_PROCESSTIMEDS_ERR_WRITE:
                        mystate->reconnect = OER_RECONNECT_ERROR;
                        reconnect();
                        break;
                }
	}
        mysql_close(&mystate->mysqldb->mysqldbconn);
	exit(EXIT_SUCCESS);
}

static void handlesignal(int signal)
{
#ifdef OER_DEBUG
        oer_debug(OER_DEBUG_NOISE, "handlesignal(%d)\n", signal);
#endif
	switch(signal) {
	case SIGINT:
	case SIGTERM:
		if(do_signals) {
			snprintf(mystate->signoff, STRINGLEN, "received %s, leaving for now but I will return!", (signal == SIGINT) ? "SIGINT" : "SIGTERM");
			quit();
		} else {
#ifdef OER_DEBUG
			oer_debug(OER_DEBUG_INFO, "handlesignal->%s received, ignoring (do_signals = 0)\n", (signal == SIGINT) ? "SIGINT" : "SIGTERM");
#endif
		}
		break;
	case SIGPIPE:
		mystate->reconnect = OER_RECONNECT_ERROR;
		strncpy(mystate->signoff, "received SIGPIPE, connecting to next server", STRINGLEN);
		break;
	case SIGUSR1:
                do_debug = (do_debug < OER_DEBUG_FLOOD) ? do_debug + 1 : OER_DEBUG_NONE;
#ifdef OER_DEBUG
                oer_debug(OER_DEBUG_NONE, "handlesignal->SIGUSR1 received, new debug level is %d\n", do_debug);
#endif
		break;
	}
}
