%{

/* snag_lex.l
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

#include "snag_parse.h"
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
^limit	{
	return LIMIT;
	}
^command {
	return COMMAND;
	}
^expect {
	return EXPECT;
	}
^ignore {
	return IGNORE;
	}
^process {
	return PROCESS;
	}
^nodf {
	return NODF;
	}
[a-zA-Z\/][a-zA-Z0-9_\-\/\.]* {
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
