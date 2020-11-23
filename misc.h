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

/* global prototype definition */
extern char *mysqldbname(char *, char *, int);
extern char *mysqlmatch(char *);
extern char *strtolower(char *);
extern char *strtoupper(char *);
extern int isansi(char *);
extern int isaction(char *);
extern int isctcp(char *);
extern int emptyline(char *);
extern int iscomment(char *);
extern int isserver(char *);
extern int isvaliduserhost(char *);
extern int isvalidhost(char *);
extern int isvalidipv4host(char *);
extern int isvalidipv6host(char *);
extern int wordcount(char *);
extern int parse(char *, int, char *, char *, int, int);
extern int oer_doquery(struct mysqldb *, char *, int level, const char *, ...);
extern int issamenickandhost(char *, char *, char *, char *);
extern int issimilarstring(char *, char *);
extern int isnumbw(int, int, int);
extern int completeban(char *, char *, int);
extern int isinitialmysql(struct mysqldb *);
extern int issamemysql(struct mysqldb *, struct mysqldb *);
extern int countchars(char *, char);
extern int countnchars(char *, char);
extern int parseprotocolmessage(char *, char *, int, char *, int, char *, int, char *, int);
extern unsigned int longestword(char *);
extern unsigned int isgoodquote(char *);
extern void striplf(char *);
extern void stripansi(char *);
extern void stripmirc(char *);
extern void stripchars(char *, char *);
extern void stripcntrl(char *);
extern void oer_debug(int, const char *, ...);
extern void secondstostring(int, char *, int);
extern void sortstring(char *);
extern void filteroutchanmodes(char *, char *, char *, int);

#ifndef HAVE_SNPRINTF
extern int snprintf(char *, size_t, const char *, ...);
#endif

#ifndef HAVE_VSNPRINTF
extern int vsnprintf(char *, size_t, const char *, va_list);
#endif


