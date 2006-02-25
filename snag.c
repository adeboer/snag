/* snag.c */

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

