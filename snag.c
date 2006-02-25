/* snag.c
 *
 *	Copyright (C) 2006 Anthony de Boer
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

#include <stdio.h>
#include "snag.h"

char *sword[] = { "OK", "WARNING", "CRITICAL", "UNDEF",
	"OOPS4", "OOPS5", "OOPS6", "OOPS7" };

main(int argc, char *argv[]) {
	int rc = 0;
	hinit(argc == 2 && strcmp(argv[1], "-b") == 0);
	setupcmd();
	if (openconfig()) yyparse();
	rc |= snagdf();
	rc |= snaginfo();
	cleancmd();
	exit(rc);
	}

