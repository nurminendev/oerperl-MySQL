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

#define __MYCRYPT_C__
#include "oer+MySQL-common.h"
#undef __MYCRYPT_C__

char salt_chars[] = { "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz" };

unsigned int getrandom(unsigned int);

int main(int argc, char *argv[])
{
	char cryptsalt[3];
        char *crypted;

	if(argc != 2) {
		exit(EXIT_FAILURE);
	}
	cryptsalt[0] = salt_chars[getrandom(strlen(salt_chars)) - 1];
	cryptsalt[1] = cryptsalt[0];
	cryptsalt[2] = '\0';
	crypted = crypt(argv[1], cryptsalt);
	printf("%s\n", crypted);
	exit(EXIT_SUCCESS);
}

unsigned int getrandom(unsigned int ceiling)
{
        unsigned int rdelta;
        unsigned int nrand;
        struct timeval tv;
        /* returns a integer between 1..ceiling */
        gettimeofday(&tv, 0);
        srand(tv.tv_usec);
        nrand = rand();
        ceiling = (ceiling > 0) ? ceiling : 1;
        rdelta = (nrand / (RAND_MAX / ceiling)) + 1;
        return rdelta;
}

