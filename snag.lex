%{
#include <string.h>
#include "snag.tab.h"
#include "snag.h"
%}

%%
[ \t]+	;
\"[^\"\n]*[\"\n] { yylval.string = strdup(yytext+1);
	if (yylval.string[yyleng-2] == '"') {
		yylval.string[yyleng-2] = '\0';
		}
	else {
		yyerror("unterminated string");
		}
	return QSTRING;
	}
\-?[0-9]* { yylval.ui = atol(yytext); return NUMBER; }
limit	{ return LIMIT; }
command { return COMMAND; }
[a-zA-Z][a-zA-Z0-9_]*	{ yylval.string = strdup(yytext); return IDENTIFIER; }
\#.*	;
\n	{ lineno++; return NL; }
.	{ return yytext[0]; }

%%

void openconfig() {
	yyin = fopen(cfile, "r");
	if (!yyin) {
		perror("snag: cannot open " cfile);
		exit(1);
		}
	}

