%{
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
	fprintf(stderr, "Command %s : %s\n", $2, $3);
	};

%%

void yyerror(char *s) {
	fprintf(stderr, "snag: %s, %s line %d\n", s, cfile, lineno);
	}

