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

#include "snag.h"

void yyerror(char *s);

int lineno = 1;

%}

%union {
	char *string;
	long ui;
	}

%token COMMAND LIMIT EXPECT IGNORE PROCESS NODF NL
%token <ui> NUMBER
%token <string> QSTRING IDENTIFIER

%%

start:	cfgline | start cfgline;

cfgline: thing NL | NL;

thing: limitcmd | commandcmd | expectcmd | ignorecmd | proccmd | nodf | setavar;

limitcmd: LIMIT IDENTIFIER NUMBER NUMBER NUMBER NUMBER
	{
	hashadd($2, $3, $4, $5, $6);
	};

commandcmd: COMMAND QSTRING QSTRING
	{
	startcmd($2, $3);
	};

expectcmd: EXPECT expectids

expectids: expid | expectids expid

expid: IDENTIFIER
	{
	procadd($1, 1, -1);
	}

ignorecmd: IGNORE ignoreids

ignoreids: ignid | ignoreids ignid

ignid: IDENTIFIER
	{
	procadd($1, 0, -1);
	}

proccmd: PROCESS IDENTIFIER NUMBER NUMBER
	{
	procadd($2, $3, $4);
	}

proccmd: PROCESS QSTRING NUMBER NUMBER
	{
	procadd($2, $3, $4);
	}

nodf: NODF IDENTIFIER
	{
	setvar($2, 1);
	}

setavar: IDENTIFIER '=' NUMBER
	{
	setvar($1, $3);
	}

%%

void yyerror(char *s) {
	fprintf(stderr, "snag: %s, %s line %d\n", s, cfile, lineno);
	}

