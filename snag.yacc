%{
#include <stdio.h>
#include <string.h>

#include "snag.h"

#define cfile "STDIN"

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
	fprintf(stderr, "Limit %s %ld %ld %ld %ld\n", $2, $3, $4, $5, $6);
	};

commandcmd: COMMAND QSTRING QSTRING
	{
	fprintf(stderr, "Command %s : %s\n", $2, $3);
	};

%%

#define LDEBUG 0

main() {
	if (LDEBUG) {
		int lv;
		char lbuf[20];
		char *lo;
		do {
			lv = yylex();
			switch (lv) {
				case COMMAND: lo = "COMMAND"; break;
				case LIMIT: lo = "LIMIT"; break;
				case NL: lo = "NL"; break;
				case NUMBER: lo = "NUMBER"; break;
				case QSTRING: lo = "QSTRING"; break;
				case 0: lo = "EOF"; break;
				default:
					if (lv < 256) {
						lbuf[0] = lv;
						lbuf[1] = '\0';
						}
					else {
						sprintf(lbuf, "TV%d", lv);
						}
					lo = lbuf;
					}
			fprintf(stderr, "yylex TOKEN %s\n", lo);
			} while (lv);
		}
	else {
		yyparse();
		}
	}

void yyerror(char *s) {
	fprintf(stderr, "snag: %s, %s line %d\n", s, cfile, lineno);
	}
