%{
#include <string.h>
#include <errno.h>

#include "snag.tab.h"
#include "snag.h"
%}

%%
[ \t]+	;
\"[^\"\n]*[\"\n] {
	yylval.string = strdup(yytext+1);
	if (yylval.string == NULL) {
		yyerror("memory allocation");
		exit(1);
		}
	if (yylval.string[yyleng-2] == '"') {
		yylval.string[yyleng-2] = '\0';
		}
	else {
		yyerror("unterminated string");
		}
	return QSTRING;
	}
\-?[0-9]* {
	yylval.ui = atol(yytext);
	return NUMBER;
	}
limit	{
	return LIMIT;
	}
command {
	return COMMAND;
	}
[a-zA-Z][a-zA-Z0-9_]* {
	yylval.string = strdup(yytext);
	if (yylval.string == NULL) {
		yyerror("memory allocation");
		exit(1);
		}
	return IDENTIFIER;
	}
\#.*	;
\n	{
	lineno++;
	return NL;
	}
.	{
	return yytext[0];
	}

%%

int openconfig() {
	yyin = fopen(cfile, "r");
	if (yyin) {
		return 1;
		}
	else if (errno == ENOENT) {
		return 0;
		}
	else {
		perror("snag: cannot open " cfile);
		exit(1);
		}
	}

void closeconfig() {
	fclose(yyin);
	}
