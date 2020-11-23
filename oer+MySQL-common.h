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

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif

#include <string.h>

#ifdef HAVE_CRYPT_H
#include <crypt.h>
#endif

#ifdef HAVE_CTYPE_H 
  #include <ctype.h> 
#endif

#ifdef HAVE_LOCALE_H
  #include <locale.h>
#endif

#include <errno.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pwd.h>
#include <signal.h>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>

#ifdef HAVE_UNAME
  #include <sys/utsname.h>
#endif

#ifdef TIME_WITH_SYS_TIME
  #include <sys/time.h>
  #include <time.h>
#else
  #ifdef HAVE_SYS_TIME_H
    #include <sys/time.h>
  #else
    #include <time.h>
  #endif
#endif

#include <sys/wait.h>

#include <mysql.h>
#include <mysql/errmsg.h>

#ifndef __MYCRYPT_C__
  #include <EXTERN.h>
  #include <perl.h>
  #include <XSUB.h>
#endif

enum NTHMODE_RETURN_VALUES {
	OER_NTHMODE_OP,
	OER_NTHMODE_DOP,
	OER_NTHMODE_VOICE,
	OER_NTHMODE_DVOICE,
	OER_NTHMODE_BAN,
	OER_NTHMODE_UNBAN,
	OER_NTHMODE_CHANMODE_WITHOUT_PARAMS,
	OER_NTHMODE_CHANMODE_WITH_PARAMS,
	OER_NTHMODE_UNKNOWN
};

enum PUBMSG_FLOOD_RETURN_VALUES {
	OER_PUBMSG_FLOOD_NORMAL,
	OER_PUBMSG_FLOOD_REPEAT,
	OER_PUBMSG_FLOOD_NONE
};

enum PARSEIRC_RETURN_VALUES {
	OER_PARSEIRC_ERR_WRITE,
	OER_PARSEIRC_ERR_PARSE,
	OER_PARSEIRC_EVERYTHING_OK
};

enum RECONNECT_TYPES {
	OER_RECONNECT_NONE,
	OER_RECONNECT_ADMIN,
	OER_RECONNECT_STONED,
	OER_RECONNECT_ERROR
};

enum WHICHCTCP_RETURN_VALUES {
	OER_WHICHCTCP_ACTION = 1,
	OER_WHICHCTCP_FINGER,
	OER_WHICHCTCP_PING,
	OER_WHICHCTCP_USERINFO,
	OER_WHICHCTCP_VERSION,
	OER_WHICHCTCP_INVALID
};

enum WHICHCOMMAND_RETURN_VALUES {
	OER_WHICHCOMMAND_TOPIC_SET,
	OER_WHICHCOMMAND_TOPIC_ADD,
	OER_WHICHCOMMAND_TOPIC_DEL,
	OER_WHICHCOMMAND_TOPIC_EDIT,
	OER_WHICHCOMMAND_TOPIC_INS,
	OER_WHICHCOMMAND_TOPIC_LIST,
	OER_WHICHCOMMAND_TOPIC_GET,
	OER_WHICHCOMMAND_TOPIC_REFRESH,
	OER_WHICHCOMMAND_TOPIC_SWAP,
	OER_WHICHCOMMAND_LOCK,
	OER_WHICHCOMMAND_LOCKU,
	OER_WHICHCOMMAND_UNLOCK,
	OER_WHICHCOMMAND_RANDOM_BANKICK,
	OER_WHICHCOMMAND_RANDOM_KICK,
	OER_WHICHCOMMAND_SYNC,
	OER_WHICHCOMMAND_SYNCALL,
	OER_WHICHCOMMAND_JUMP,
	OER_WHICHCOMMAND_QUIT,
	OER_WHICHCOMMAND_OP,
	OER_WHICHCOMMAND_DOP,
	OER_WHICHCOMMAND_VOICE,
	OER_WHICHCOMMAND_DEVOICE,
	OER_WHICHCOMMAND_BAN,
	OER_WHICHCOMMAND_UNBAN,
	OER_WHICHCOMMAND_BANKICK,
	OER_WHICHCOMMAND_KICK,
	OER_WHICHCOMMAND_INVITE,
	OER_WHICHCOMMAND_MODE,
	OER_WHICHCOMMAND_MASSMESSAGE,
	OER_WHICHCOMMAND_HELP,
	OER_WHICHCOMMAND_LIST,
	OER_WHICHCOMMAND_INFO,
	OER_WHICHCOMMAND_RAW,
	OER_WHICHCOMMAND_UPTIME,
	OER_WHICHCOMMAND_ADD,
	OER_WHICHCOMMAND_DEL,
	OER_WHICHCOMMAND_EDIT,
	OER_WHICHCOMMAND_SAY,
	OER_WHICHCOMMAND_LAST,
	OER_WHICHCOMMAND_QUOTE,
	OER_WHICHCOMMAND_SEEN,
	OER_WHICHCOMMAND_NSTATS,
	OER_WHICHCOMMAND_ACTION,
	OER_WHICHCOMMAND_LOGON,
	OER_WHICHCOMMAND_LOGOFF,
	OER_WHICHCOMMAND_WALL,
        OER_WHICHCOMMAND_CLONECHANNEL,
        OER_WHICHCOMMAND_SSTATS,
        OER_WHICHCOMMAND_DATE,
        OER_WHICHCOMMAND_USERCOPY,
        OER_WHICHCOMMAND_CLEARBANS,
        OER_WHICHCOMMAND_CYCLE,
        OER_WHICHCOMMAND_BSTATS,
        OER_WHICHCOMMAND_EXT,
        OER_WHICHCOMMAND_DBCLOSE,
        OER_WHICHCOMMAND_DBCONNECT,	
        OER_WHICHCOMMAND_RESET,
        OER_WHICHCOMMAND_CHANINFO,
        OER_WHICHCOMMAND_QUEUESTATS,
	OER_WHICHCOMMAND_INVALID
};

enum PROCESSTIMEDS_RETURN_VALUES {
	OER_PROCESSTIMEDS_ERR_WRITE,
	OER_PROCESSTIMEDS_EVERYTHING_OK
};

enum ESTABLISHCONNECTION_RETURN_VALUES {
	OER_ESTABLISHCONNECTION_EVERYTHING_OK
};

enum SERVERCONNECTION_RETURN_VALUES {
	OER_SERVERCONNECTION_ERR_SOCKET,
	OER_SERVERCONNECTION_ERR_GETHOSTNAME,
	OER_SERVERCONNECTION_ERR_GETADDRINFO,
	OER_SERVERCONNECTION_ERR_GETADDRINFO_RETRY,
	OER_SERVERCONNECTION_ERR_GETSERVER_RETRY,
	OER_SERVERCONNECTION_ERR_BIND,
	OER_SERVERCONNECTION_ERR_CONNECT,
	OER_SERVERCONNECTION_ERR_CONNECT_RETRY,
	OER_SERVERCONNECTION_ALREADY_CONNECTED,
	OER_SERVERCONNECTION_EVERYTHING_OK
};

enum REGISTERCONNECTION_RETURN_VALUES {
        OER_REGISTERCONNECTION_ERR_GETPWUID,
        OER_REGISTERCONNECTION_ERR_WRITE,
        OER_REGISTERCONNECTION_ERR_HANDLESERVERDATA,
        OER_REGISTERCONNECTION_ALREADY_REGISTERED,
        OER_REGISTERCONNECTION_TIMEOUT,
        OER_REGISTERCONNECTION_ERROR,
        OER_REGISTERCONNECTION_EVERYTHING_OK
};

enum HANDLESERVERDATA_RETURN_VALUES {
	OER_HANDLESERVERDATA_ERR_READ,
	OER_HANDLESERVERDATA_ERR_NOTHING_READ,
	OER_HANDLESERVERDATA_ERR_WRITE,
	OER_HANDLESERVERDATA_EVERYTHING_OK
};

enum OER_DEBUG_LEVELS {
	OER_DEBUG_NONE = 0,
	OER_DEBUG_FATAL = 1,
	OER_DEBUG_ERROR = 2,
	OER_DEBUG_WARNING = 3,
	OER_DEBUG_INFO = 4,
	OER_DEBUG_NOISE = 8,
	OER_DEBUG_FLOOD = 9
};

enum OER_BAN_TYPES {
        OER_BAN_TYPE_HOST = 1,
        OER_BAN_TYPE_USER,
        OER_BAN_TYPE_USER_HOST,
        OER_BAN_TYPE_NICK,
        OER_BAN_TYPE_NICK_HOST,
        OER_BAN_TYPE_NICK_USER,
        OER_BAN_TYPE_NICK_USER_HOST,
        OER_BAN_TYPE_INVALID
};

enum OER_TIMED_TYPES {
        OER_TIMED_TYPE_NORMAL = 1,
        OER_TIMED_TYPE_KICK,
        OER_TIMED_TYPE_USERHOST,
        OER_TIMED_TYPE_INVALID
};

enum OER_TIMED_PRIORITIES {
        /* lowest priority first */
        OER_TIMED_PRIORITY_WALL = 1,
        OER_TIMED_PRIORITY_FLOOD,
        OER_TIMED_PRIORITY_CTCP,
        OER_TIMED_PRIORITY_NOTICE,
        OER_TIMED_PRIORITY_PRIVMSG,
        OER_TIMED_PRIORITY_ADVERT,
        OER_TIMED_PRIORITY_NORMAL,
        OER_TIMED_PRIORITY_CHANNEL_HANDLING,
        OER_TIMED_PRIORITY_CHANNEL_PROTECTION,
        OER_TIMED_PRIORITY_CHANNEL_INVALID
};

enum OER_FLAGS_TYPE {
        OER_FLAGS_TYPE_GLOBAL,
        OER_FLAGS_TYPE_CHANNEL,
        OER_FLAGS_TYPE_ADMIN,
        OER_FLAGS_TYPE_USER
};

#define OER_FLAGS_GLOBAL "dfglmnopqsS"
#define OER_FLAGS_CHANNEL "!aAbcDefFGklLmMnNoOpPqrRsSuUtTvVwxX"
#define OER_FLAGS_ADMIN "dn"
#define OER_FLAGS_USER "!adfmnovrsx"

#define numofparams(x) (wordcount(x) - 1)

/* from glib.h */
#define ABS(a)         (((a) < 0) ? -(a) : (a))

#define OER_LIMIT_SERVER_PORT_MIN 1
#define OER_LIMIT_SERVER_PORT_MAX 65536
#define OER_LIMIT_SERVER_MODES_MIN 1
#define OER_LIMIT_SERVER_MODES_MAX 6
#define OER_LIMIT_SERVER_PING_MIN 10
#define OER_LIMIT_SERVER_PING_MAX 86400
#define OER_LIMIT_SERVER_PROTOPER_MIN 0
#define OER_LIMIT_SERVER_PROTOPER_MAX 1
#define OER_LIMIT_BANVARS_AUTOREJOIN_MIN 0
#define OER_LIMIT_BANVARS_AUTOREJOIN_MAX 86400
#define OER_LIMIT_BANVARS_PARTREJOIN_MIN 0
#define OER_LIMIT_BANVARS_PARTREJOIN_MAX 86400
#define OER_LIMIT_BANVARS_PUBLICFLOOD_MIN 0
#define OER_LIMIT_BANVARS_PUBLICFLOOD_MAX 86400
#define OER_LIMIT_BANVARS_PUBLICFLOODREPEAT_MIN 0
#define OER_LIMIT_BANVARS_PUBLICFLOODREPEAT_MAX 86400
#define OER_LIMIT_BANVARS_BADWORD_MIN 0
#define OER_LIMIT_BANVARS_BADWORD_MAX 86400
#define OER_LIMIT_BANVARS_BADNICK_MIN 0
#define OER_LIMIT_BANVARS_BADNICK_MAX 86400
#define OER_LIMIT_BANVARS_NORMALBAN_MIN 0
#define OER_LIMIT_BANVARS_NORMALBAN_MAX 86400
#define OER_LIMIT_CHANNEL_CHANLIMIT_MIN 1
#define OER_LIMIT_CHANNEL_CHANLIMIT_MAX 99999
#define OER_LIMIT_FLOODVARS_REPEAT_EXPIRE_MIN 1
#define OER_LIMIT_FLOODVARS_REPEAT_EXPIRE_MAX 86400
#define OER_LIMIT_FLOODVARS_REPEAT_LIMIT_MIN 1
#define OER_LIMIT_FLOODVARS_REPEAT_LIMIT_MAX 1000
#define OER_LIMIT_FLOODVARS_INTERVAL_MIN 1
#define OER_LIMIT_FLOODVARS_INTERVAL_MAX 86400
#define OER_LIMIT_FLOODVARS_LINES_MIN 1
#define OER_LIMIT_FLOODVARS_LINES_MAX 1000
#define OER_LIMIT_FLOODVARS_CHARS_MIN 1
#define OER_LIMIT_FLOODVARS_CHARS_MAX 100000
#define OER_LIMIT_FLOODVARS_NICKFLOOD_EXPIRE_MIN 1
#define OER_LIMIT_FLOODVARS_NICKFLOOD_EXPIRE_MAX 86400
#define OER_LIMIT_FLOODVARS_NICKFLOOD_LIMIT_MIN 1
#define OER_LIMIT_FLOODVARS_NICKFLOOD_LIMIT_MAX 1000
#define OER_LIMIT_LISTCOMMAND_STARTPOS_MIN 1
#define OER_LIMIT_LISTCOMMAND_STARTPOS_MAX 1000
#define OER_LIMIT_TOPICS_MIN 1
#define OER_LIMIT_TOPICS_MAX OER_TOPICS
#define OER_LIMIT_SHOWLAST_MIN 1
#define OER_LIMIT_SHOWLAST_MAX OER_LAST_MAX
#define OER_LIMIT_SHOWSEEN_MIN 1
#define OER_LIMIT_SHOWSEEN_MAX OER_SEEN_MAX
#define MYSQL_BANNED_CHARS "/.#-"
#define OER_MAINLOOP_TIMEOUT_SECS 1
#define OER_MAINLOOP_TIMEOUT_USECS 0
/* only change this if you are 100% sure what you are doing */
#define OER_ACTIONS_PER_MAINLOOP 2
#define OER_KICKS_PER_MAINLOOP 1
#define OER_KICK_INTERVAL 2
#define OER_GN_INTERVAL 10
#define OER_NICKS_PER_USERHOST 5
/* multiple kicks aren't supported for backwards compatibility */
#define OER_NICKS_PER_KICK 1
#define OER_DEFAULT_DELIM "::"
#define OER_DEFAULT_DELIM_LEN 2
#define OER_QUOTE_MIN_LENGTH 20
#define OER_QUOTE_MAX_LENGTH 200
#define OER_QUOTE_MAX_WORD_LENGTH 20
#define OER_WAIT_BETWEEN_CONNECTION_ATTEMPTS 15
#define OER_JOINS 20
#define OER_PARTS 20
#define OER_PART_EXPIRE 1800
#define OER_BURST_JOINS OER_JOINS / 3
#define OER_BURST_JOINS_LIMIT OER_JOINS / 2
#define OER_DELAY_BETWEEN_REJOINS 10
#define OER_DELAY_BETWEEN_REJOINS_CYCLE 1
#define OER_DELAY_CHANMODES_AFTER_JOIN 10
#define OER_DELAY_BETWEEN_RECORD_CHECKS 120
#define OER_AUTO_REJOIN_TIME 3
#define OER_QUOTE_INTERVAL 300
#define OER_ADVERT_INTERVAL 300
#define OER_Q_QUERY_INTERVAL 10
#define OER_ALLOWED_PARTS OER_PARTS / 5
#define OER_RECONNECT_DELAY 30
#define OER_NETJOIN_DELAY 15
#define OER_PUBMSGS 20
#define OER_NICKCHANGES 20
#define OER_SERVER_IS_STONED_FOR 300
#define OER_LINENOISE_INTERVAL_MIN 0
#define OER_MASSMODE_LIMIT 3
#define OER_LAST 3
#define OER_LAST_MAX 10
#define OER_LAST_MAX_LENGTH 120
#define OER_SEEN 3
#define OER_SEEN_MAX 10
#define OER_TOPICS 20
#define OER_TOPICLEN 250
#define TOPIC_SEPARATOR " | "
#define CONNECTION_STATE_PREFIXES "^~+=-"
#define OER_PERMBANS 25
#define OER_LOGON_TIMEOUT 1800
#define OER_REGISTERCONNECTION_TIMEOUT 600
#define OER_MYSQL_CONNECTION_RETRY 60
#define OER_MYSQL_CONNECTION_RETRY_AFTER_LOSS 600
#define OER_MYSQL_TIMESTAMP_CHECK_INTERVAL 300
#define OER_BANVARS_AUTO_REJOIN 30
#define OER_BANVARS_PART_REJOIN 600
#define OER_BANVARS_PUBLIC_FLOOD 300
#define OER_BANVARS_PUBLIC_FLOOD_REPEAT 600
#define OER_BANVARS_BAD_WORD 3600
#define OER_BANVARS_BAD_NICK 3600
#define OER_BANVARS_NORMAL_BAN 3600
#define OER_ALLOWED_QUOTE_TIME_USEC 600000
#define Q_NICK "Q"
/* Define if you want to show server idle percentage */
/* #undef OER_UPTIME_SHOW_IDLE */
#define PUBMSG_FLOOD_LINES OER_PUBMSGS / 2
#define PUBMSG_FLOOD_CHARS OER_PUBMSGS * 80
#define PUBMSG_FLOOD_INTERVAL 60
#define PUBMSG_FLOOD_REPEAT 3
#define PUBMSG_FLOOD_REPEAT_EXPIRE 1800
#define NICKFLOOD_EXPIRE 7200
#define NICKFLOOD_CHANGES 5
#define TXT_BUFFER 1024
/* this is the ircd buffer size */
#define IRCD_BUFFER_LENGTH 512
#define WRITE_BUFFER_LENGTH IRCD_BUFFER_LENGTH
/* this could be larger, i don't see any reason why though */
#define READ_BUFFER_LENGTH IRCD_BUFFER_LENGTH
/* we leave 80 bytes for header strings */
#define WRITE_BUFFER_HEADER_LENGTH 80
#define MICROSTRINGLEN 32
#define TINYSTRINGLEN 32
#define MIDSTRINGLEN 64
#define STRINGLEN 128
#define BIGSTRINGLEN 256
#define HUGESTRINGLEN (WRITE_BUFFER_LENGTH - WRITE_BUFFER_HEADER_LENGTH)
#define CHANLEN 50
#define FLAGLEN 64
#define HOSTLEN 63
#define USERLEN 16
#define NICKLEN 20
#define IDENTLEN NICKLEN
#define USERHOSTLEN HOSTLEN + USERLEN + NICKLEN + 2
#define OER_MAX_LIST_LINES 5
#define OER_HELP "For the oer+MySQL user manual go to http://oer.equnet.org/um.php"
#define OER_SERVER_IS_STONED "stoned server"
#define OER_INTERNAL_DYNAMIC_ADMIN "*INTDYN*"
#define DEFAULT_NICKBK_MESSAGE "prohibited nick"
#define DEFAULT_WORDBK_MESSAGE "word ban-kick triggered, have a nice life..."
#define DBSTATUS_MSG "At least one of the executed database queries failed, more \
details possibly available on the oer+MySQL console"
#define OER_REALNAME "http://oer.equnet.org"
#define OER_UNKNOWN "unknown"
#define OER_VERSION "oer+MySQL version 1.0-42 (oerperl+MySQL 1.0.1)"
#define OER_COPYRIGHT1 "Copyright (C) 2000-2004 EQU <equ@equnet.org>"
#define OER_COPYRIGHT2 "oer+MySQL comes with ABSOLUTELY NO WARRANTY; for details see the\n\
GNU General Public License (COPYING) provided with this distribution.\n\
This is free software, and you are welcome to redistribute it under\n\
certain conditions; again see COPYING for details.\n"
#define OERPERL_BLOCK_OER_COMMAND 1

#ifndef __MYCRYPT_C__
struct scripttimer {
	time_t interval;
	time_t lastrun;
	SV *callback;
	struct scripttimer *prev;
	struct scripttimer *next;
};

struct commandhandler {
	char command[TINYSTRINGLEN + 1];
	SV *callback;
	struct commandhandler *prev;
	struct commandhandler *next;
};

struct eventhandler {
	char eventtype[MICROSTRINGLEN + 1];
	SV *callback;
	struct eventhandler *prev;
	struct eventhandler *next;
};

struct script {
	char filename[BIGSTRINGLEN + 1];
	struct eventhandler *eventhandlers;
	struct commandhandler *commandhandlers;
	struct scripttimer *timers;
	SV *startup_callback;
	SV *shutdown_callback;
	struct script *prev;
	struct script *next;
};
#endif

struct mysqldb {
	int isdead;
	time_t isalive;
	time_t tod;
        char mysqldbdbhost[HOSTLEN + 1];
        char mysqldbdbname[TINYSTRINGLEN + 1];
        char mysqldbdbuser[TINYSTRINGLEN + 1];
        char mysqldbdbpw[TINYSTRINGLEN + 1];
        MYSQL mysqldbconn;
};

struct floodvars {
	int repeat_expire;
	int repeat_limit;
	int interval;
	int lines;
	int chars;
	int nickflood_expire;
	int nickflood_changes;
};

struct banvars {
	int auto_rejoin;
	int part_rejoin;
	int public_flood;
	int public_flood_repeat;
	int bad_word;
	int bad_nick;
	int normal_ban;
};

struct locked {
	int locked;
	int unlocked;
	int auto_unlock;
	time_t lastkick;
	char reason[STRINGLEN + 1];
	char nick[NICKLEN + 1];
	char host[USERHOSTLEN + 1];
};

struct qauth {
	int hasauth;
	int isonline;
	time_t lastquery;
	int authed;
	char q[USERHOSTLEN + 1];
	char name[NICKLEN + 1];
	char password[TINYSTRINGLEN + 1];
};

struct chanuser {
	char *nick;
	char *userhost;
	int hostquery;
	int ircop;
	int chanop;
	int voice;
	int tobek;
	int friend;
	time_t isflood_last;
        int isflood_lines;
        int isflood_chars;
	struct chanuser *prev;
	struct chanuser *next;
};

struct botuser {
	char *handle;
	char *options;
	char *password;
	struct maskstruct *firstmask;
	struct botuser *prev;
	struct botuser *next;
};

struct authed {
	time_t at;
	int forced_logoff;
	char *userhost;
	char *handle;
	struct authed *prev;
	struct authed *next;
};

struct pubmsg {
	time_t at;
	char *nick;
	char *userhost;
	char *message;
	struct pubmsg *prev;
	struct pubmsg *next;
};

struct nickchange {
        time_t at;
        char *userhost;
        struct nickchange *prev;
        struct nickchange *next;
};

struct join {
	time_t at;
	char *nick;
	char *userhost;
	struct join *prev;
	struct join *next;
};

struct part {
	time_t at;
	int valid;
	char *nick;
	char *userhost;
	struct part *prev;
	struct part *next;
};

struct mmode {
        time_t at;
        char *command;
        char *target;
        struct mmode *prev;
        struct mmode *next;
};

struct timed {
        time_t at;
        int type;
        int prio;
        char *channel;
        char *command;
        struct timed *prev;
        struct timed *next;
};

struct maskstruct {
	char *mask;
	char *optstring1;
	char *optstring2;
	struct maskstruct *prev;
	struct maskstruct *next;
};

struct advert {
        char *to;
        char *message;
        struct advert *prev;
        struct advert *next;
};

struct registered {
        int begin;
        int pass;
        int nick;
        int user;
        int done;
};

struct last_action {
        time_t ts;
        unsigned int barrier;
};

struct channelban {
        char *ban;
        char *setby;
        time_t at;
        struct channelban *next;
        struct channelban *prev;
};

struct syncrequest {
	char target[CHANLEN + 1];
	char nick[NICKLEN + 1];
	struct syncrequest *next;
	struct syncrequest *prev;
};

struct server {
	char serverhost[HOSTLEN + 1];
	char serverhost_r[HOSTLEN + 1];
	int serverport;
	int servermodes;
	int pingfrequency;
	int protected_ircops;
	int used;
	int connected;
	struct registered registered;
	int stoned;
	int linenoise;
	int rx;
	int tx;
	time_t linkup;
	char password[TINYSTRINGLEN + 1];
	time_t lastping;
	time_t registerconnection_start;
	struct server *prev;
	struct server *next;
};

struct channel {
	char name[CHANLEN + 1];
	char mode[CHANLEN + 1];
	char key[CHANLEN + 1];
	char limit[CHANLEN + 1];
	char chanflags[FLAGLEN + 1];
	int nickcount;
	int setchanmode;
	int joined;
	int joining;
	int i_am_op;
	int topic_change;
	int topic_reset;
	int synced;
        int bantype;
	int allhostsknown;
        int whoquery;
	int requestop_now;
	int chankeyisset;
        int chanlimitisset;
	time_t rejoin_at;
	time_t last_quote;
	time_t last_advert;
	time_t join_ts;
	time_t userts;
	time_t userauthedsts;
	time_t requestop;
	struct join *joins;
	struct part *parts;
	struct pubmsg *pubmsgs;
	struct nickchange *nickchanges;
	struct locked locked;
        struct floodvars floodvars;
        struct banvars banvars;
	struct chanuser *nicks;
	struct maskstruct *nickbks;
	struct maskstruct *wordbks;
	struct botuser *users;
	struct authed *autheds;
	struct advert *adverts;
	struct mmode *mmodes;
	struct channelban *channelbans;
	struct channel *prev;
	struct channel *next;
};

struct state {
	char config[STRINGLEN + 1];
	/* this is our current nick, might be temporarily set
	to getnick when attempting to change nick */
        char nick[NICKLEN + 1];
        char altnick[NICKLEN + 1];
        char getnick[NICKLEN + 1];
        char user[USERLEN + 1];
        char host[HOSTLEN + 1];
	char mode[CHANLEN + 1];
	char signoff[STRINGLEN + 1];
	char preferredserver[HOSTLEN + 1];
	char realname[STRINGLEN + 1];
	char flags[FLAGLEN + 1];
	char ident[IDENTLEN + 1];
	char adminsfrom[IDENTLEN + 1];
        char usersfrom[IDENTLEN + 1];
	char prefix[TINYSTRINGLEN + 1];
	char state[STRINGLEN + 1];
	char proxysetup[STRINGLEN + 1];
        char vhost[HOSTLEN + 1];
	int customuser;
	int use_altnick;
	int sockfd;
	int loopforever;
	int newnick;
	int newmode;
	int netjoining;
	int bailout;
	int quitting;
	int reconnect;
	time_t now;
	time_t startup;
	time_t postnj_checks_at;
	time_t admints;
	time_t adminauthedsts;
	time_t last_linenoise;
	time_t last_gn;
	time_t last_ts_check;
	struct qauth qauth;
	struct authed *autheds;
	struct botuser *admins;
	struct channel *channels;
	struct maskstruct *trusted;
        struct maskstruct *services;
	struct server *servers;
	struct server *current_server;
	struct timed *timeds;
        struct syncrequest *syncs;
        struct mysqldb *mysqldb;
        struct mysqldb *mysqladmins;
        struct mysqldb *mysqlusers;
	struct last_action last_action;
	struct script *scripts;
};
