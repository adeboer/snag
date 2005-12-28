/* snag.c */

#include <stdio.h>
#include "snag.h"

char *sword[] = { "OK", "WARNING", "CRITICAL", "UNDEF",
	"OOPS4", "OOPS5", "OOPS6", "OOPS7" };

main() {
	int rc = 0;
	hinit();
	rc |= snagdf();
	rc |= snaginfo();
	exit(rc);
	}

