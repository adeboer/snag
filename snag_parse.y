%{

/* snag_parse.y
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
#include <string.h>

#include "snag.h"

void yyerror(char *s);

int lineno = 1;

%}

%union {
	char *string;
	long ui;
	}

%token COMMAND LIMIT NL
%token <ui> NUMBER
%token <string> QSTRING IDENTIFIER

%%

start:	thing | start thing;

thing: limitcmd NL | commandcmd NL | NL;

limitcmd: LIMIT IDENTIFIER NUMBER NUMBER NUMBER NUMBER
	{
	hashadd($2, $3, $4, $5, $6);
	};

commandcmd: COMMAND QSTRING QSTRING
	{
	startcmd($2, $3);
	};

%%

void yyerror(char *s) {
	fprintf(stderr, "snag: %s, %s line %d\n", s, cfile, lineno);
	}

