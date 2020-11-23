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

/* global prototype definitions */
extern char *safeban(struct channel *, char *, char *);
extern char *getfirsthostless(struct channel *, char *);
extern char *getkickreason(char *);
extern char getmodepol(char *, char);
extern int topiccount(struct channel *);
extern int settopic(struct channel *);
extern int setnewtopic(struct channel *, char *, char *);
extern int addnewtopic(struct channel *, char *, char *);
extern int insertnewtopic(struct channel *, char *, char *, int);
extern int edittopic(struct channel *, char *, char *, int);
extern int gettopic(struct channel *, char *);
extern int listtopic(struct channel *, char *, int);
extern int deltopics(struct channel *);
extern int swaptopic(struct channel *, int, int);
extern int partcount(struct channel *, char *, char *);
extern int admincount(void);
extern int loadconf(char *);
extern int parseconf(char *);
extern int getadmins(void);
extern int getadminauhteds(void);
extern int getusers(struct channel *);
extern int getuserauhteds(struct channel *);
extern int updateadmints(void);
extern int updateadminauthedsts(void);
extern int updateuserts(struct channel *);
extern int updateuserauthedsts(struct channel *);
extern int initenv(void);
extern int burstjoins(struct channel *);
extern int isq(char *);
extern int isservice(char *);
extern int istrusted(char *);
extern int isme(char *);
extern int isop(struct channel *, char *);
extern int isvoice(struct channel *, char *);
extern int isvoicenow(struct channel *, char *);
extern int isvalidlogon(struct channel *, char *);
extern int isopa(char *);
extern int ishostless(char *);
extern int delopa(char *);
extern int isatleastopnow(struct channel *, char *, char *);
extern int isatleastop(struct channel *, char *, char *);
extern int isallowedop(struct channel *, char *);
extern int isadmin(struct channel *, char *);
extern int isonchan(struct channel *, char *);
extern int issameuser(struct channel *, char *, char *);
extern int isopnow(struct channel *, char *);
extern int nickchange(struct channel *, char *, char *);
extern int isflood(struct channel *, char *, char *, char *);
extern int haschanflags(struct channel *, char*);
extern int hasadminflags(char *, char *);
extern int ispermban(struct channel *, char *);
extern int ismoderated(struct channel *);
extern int permbancount(struct channel *);
extern int nthmode(char *, int);
extern int userleft(struct channel *, char *, char *);
extern int changeuser(struct channel *, char *, int, int, int, int);
extern int whichctcp(char *);
extern int whichcommand(char *, int);
extern int gettobek(struct channel *, char *);
extern int checkforansi(struct channel *, char *, char *, char *);
extern int checkfornickflood(struct channel *, char *, char *);
extern int checkforflood(struct channel *, char *, char *, char *);
extern int checkforbadword(struct channel *, char *, char *, char *);
extern int checkforbadnick(struct channel *, char *, char *);
extern int checkforautorejoin(struct channel *, char *, char *);
extern int checkforpartrejoin(struct channel *, char *, char *);
extern int checkforaction(struct channel *, char *, char *, int);
extern int delnickbks(char *, char *, int);
extern int delwordbks(char *, char *, int);
extern int uptime(time_t *, time_t *);
extern int deladmin(char *, int);
extern int deladminmask(char *, char *);
extern int deluser(struct channel *, char *);
extern int delusermasks(struct channel *, char *, char *, int);
extern int deltrusted(char *);
extern int delservice(char *);
extern int delscript(char *);
extern int delchannel(struct channel *);
extern int deladverts(struct channel *, char *, int);
extern int delchankey(struct channel *);
extern int delchanlimit(struct channel *);
extern int delchanmode(struct channel *);
extern int getondiskmsgcount(struct channel *, char *);
extern int getondiskjoincount(struct channel *, char *);
extern int getrandommsg(struct channel *, char *, int, char *, int, int);
extern int getjoincount(struct channel *, char *);
extern int delserver(char *, int, int, int, int, int);
extern int setpassword(struct channel *, char *, char *);
extern int logoff(struct channel *, char *, int);
extern int logon(struct channel *, struct botuser *, char *, char *, int);
extern int timed_cmp(struct timed *, struct timed *);
extern int isfriend(struct channel *, char *, char *);
extern int deltaflags(struct channel *, char *, char);
extern int isvalidchannel(char *);
extern int hasuserflags(struct channel *, char *, char *, char *);
extern int noexpiredlogons(struct channel *);
extern int delchannelban(struct channel *, char *);
extern int ischanopless(struct channel *);
extern int joininprogress(void);
extern int dbconnect(struct mysqldb *);
extern int dbping(struct mysqldb *);
extern int dbpingall(void);
extern int delpermbans(struct channel *, char *, int);
extern int getmmodecount(struct channel *);
extern int gettimedcount(void);
extern int deltopic(struct channel *, char *, int, char *, char *, char *);
extern int cleanautheds(struct channel *, int, int);
extern unsigned int getrandom(unsigned int);
extern struct authed *addnewauthed(struct channel *, time_t, char *, char *);
extern struct botuser *addbotuser(char *, char *);
extern struct botuser *addnewadmin(char *, char *);
extern struct botuser *addnewuser(struct channel *, char *, char *);
extern struct botuser *usercopy(struct channel *, char *, struct channel *, char *);
extern struct channel *addnewchannel(char *);
extern struct channel *getchptr(char *);
extern struct channelban *addnewchannelban(struct channel *, char *, char *, time_t);
extern struct chanuser *userjoined(struct channel *, char *, char *, int, int, int, int);
extern struct chanuser *getcuptr(struct channel *, char *);
extern struct state *emptystate(void);
extern struct server *addnewserver(char *, int, int, int, int, int, char *);
extern struct server *getserver(void);
extern struct advert *addnewadvert(struct channel *, char *, char *);
extern struct maskstruct *addnewmask(char *);
extern struct maskstruct *addnewnickbk(struct channel *, char *, char *, char *);
extern struct nickchange *addnewnickchange(struct channel *, char *);
extern struct maskstruct *addnewwordbk(struct channel *, char *, char *, char *);
extern struct maskstruct *addnewtrusted(char *);
extern struct maskstruct *addnewservice(char *);
extern struct script *addnewscript(char *);
extern struct eventhandler *addneweventhandler(struct script *, char *, SV *);
extern struct scripttimer *addnewscripttimer(struct script *, time_t, SV *);
extern struct commandhandler *addnewcommandhandler(struct script *, char *, SV *);
extern struct maskstruct *addnewkickreason(char *);
extern struct maskstruct *addnewusermask(struct channel *, char *, char *);
extern struct maskstruct *addnewadminmask(char *, char *);
extern struct maskstruct *editmask(struct maskstruct *, char *, char *);
extern struct maskstruct *isnickbk(struct channel *, char *);
extern struct maskstruct *iswordbk(struct channel *, char *);
extern struct pubmsg *addnewpubmsg(struct channel *, time_t, char *, char *, char *);
extern struct part *addnewpart(struct channel *, time_t, char *, char *);
extern struct join *addnewjoin(struct channel *, time_t, char *, char *);
extern struct timed *addnewtimed(time_t, int, int, char *, char *, char *, char *);
extern struct chanuser *getrandomuser(struct channel *);
extern struct timed *timed_new(struct channel *, time_t, int, int, char *);
extern struct mmode *mmode_new(struct channel *, time_t, char *, char *);
extern struct channel *clonechannel(char *, char *);
extern time_t lastoff(struct channel *, char *, char *);
extern time_t getadmints(void);
extern time_t getadminauhtedsts(void);
extern time_t getuserts(struct channel *);
extern time_t getuserauhtedsts(struct channel *);
extern void sendadverts(struct channel *, char *, char *);
extern void resetparts(struct channel *, char *, char *);
extern void initpubmsgs(struct channel *);
extern void initchannelbans(struct channel *);
extern void freenickchanges(struct channel *, char *);
extern void freepubmsguser(struct channel *, char *, char *);
extern void lockchan(struct channel *, char *, int, char *, char *);
extern void unlockchan(struct channel *, char *, char *);
extern void initchannel(struct channel *);
extern void initall(void);
extern void initnicks(struct channel *);
extern void initmmodes(struct channel *);
extern void initmmodesfornick(struct channel *, char *);
extern void changemmodesfornick(struct channel *, char *, char *);
extern void initparts(struct channel *);
extern void initjoins(struct channel *);
extern void initnickbks(struct channel *);
extern void initwordbks(struct channel *);
extern void initusers(struct channel *);
extern void checkstoned(void);
extern void setuserhost(char *, char *);
extern void sethostquerystatus(char *, int);
extern void banuser(struct channel *, time_t, char *);
extern void unbanuser(struct channel *, time_t, char *);
extern void kickuser(struct channel *, time_t, char *, char *);
extern void sendreply(char *, int, int, int, char *);
extern void sendwall(char *, struct channel *, int, char *);
extern void sendchannelnotice(struct channel *, int, char *);
extern void addcommand(struct channel *, char *, int, char *, char *, char *);
extern void delcommand(struct channel *, char *, int, char *, char *, char *);
extern void editcommand(struct channel *, char *, int, char *, char *, char *);
extern void extcommand(struct channel *, char *, int, char *, char *, char *);
extern void listcommand(struct channel *, char *, int, char *, char *, char *);
extern void logoffcommand(struct channel *, char *, int, char *, char *, char *);
extern void logoncommand(struct channel *, char *, int, char *, char *, char *);
extern void changetobek(struct channel *, char *, int);
extern void syncvoices(struct channel *);
extern void syncops(struct channel *);
extern void syncpermbans(struct channel *);
extern void syncnickbks(struct channel *);
extern void syncuserhosts(void);
extern void processenv(void);
extern void processlock(struct channel *);
extern void processnetjoin(struct channel *);
extern void mmodes2timeds(void);
extern void joinchannel(struct channel *);
extern void updatelast(struct channel *, char *, char *, char *);
extern void updateseen(struct channel *, char *, char *);
extern void showlast(struct channel *, char *, int, char *);
extern void showseen(struct channel *, char *, int, char *);
extern void setchanmode(struct channel *);
extern void setchankey(struct channel *, char *);
extern void channelsync(struct channel *);
extern void cleartobek(struct channel *);
extern void getnthmode(char *, int, char *);
extern void massmessage(char *, char *);
extern void quit(void);
extern void parsectcp(char *, char *, int, char *);
extern void nstats(struct channel *, char *, int, char *);
extern void wall(struct channel *, char *, char *, char *);
extern void listchanusers(void);
extern void timed_del(struct timed *);
extern void mmode_del(struct channel *, struct mmode *);
extern void validateflags(char *, char *, int);
extern void resetfloodcounters(struct channel *, char *);
extern void dbclose(void);
extern void resetcommand(struct channel *, char *, int, char *, char *, char *);
extern void syncadmins(void);
extern void syncusers(struct channel *);
extern void sendadminsyncrequest(void);
extern void sendusersyncrequest(struct channel *);
extern void addnewsyncrequest(char *, char *);
extern void sendsyncrequests(void);
