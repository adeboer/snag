/* snag.c
 *
 *	Copyright (C) 2006,2008 Anthony de Boer
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of version 2 of the GNU General Public License as
 *	published by the Free Software Foundation.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
 *	USA
 */

#include "snag.h"

char *statusword(int sw) {
	char *rw;
	switch(sw) {
		case 0: rw = "OK"; break;
		case 1: rw = "WARNING"; break;
		case 2: rw = "CRITICAL"; break;
		case 3: rw = "UNDEF"; break;
		default: rw = "?OOPS";
		}
	return rw;
	}

void usage() {
	fprintf(stderr, "Usage: snag [-b]\n");
	exit(2);
	}

int main(int argc, char *argv[]) {
	int rc = 0;
	int hflag;
	if (argc == 1) {
		hflag = 0;
		}
	else if (argc == 2 && strcmp(argv[1], "-b") == 0) {
		hflag = 1;
		}
	else {
		usage();
		}
	hinit(hflag);
	setupcmd();
	if (openconfig()) yyparse();
	rc |= snagdf();
	rc |= snaginfo();
	cleancmd();
	exit(rc);
	}

