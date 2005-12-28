/* thresh.c */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "snag.h"

#define HASHSIZE 1009

int hash(char *s) {
	int h = 0;
	while(*s) {
		h += *s++;
		}
	return (h % HASHSIZE);
	}

struct hnode {
	char *name;
	struct hnode *next;
	long lcrit, lwarn, hwarn, hcrit;
	};

struct hnode *htab[HASHSIZE];

struct hnode *hasher(char *s, int make) {
	int h = hash(s);
	struct hnode *p = htab[h];
	char *s2;
	while(p) {
		if (strcmp(s, p->name) == 0) {
			return p;
			}
		p = p->next;
		}
	if (make && (s2 = strdup(s)) && (p = (struct hnode *)calloc(1, sizeof (struct hnode)))) {
		p->name = s2;
		p->next = htab[h];
		htab[h] = p;
		}
	return p;
	}

void hashadd(char *s, long lcrit, long lwarn, long hwarn, long hcrit) {
	struct hnode *hent = hasher(s, 1);
	if (!hent) {
		fprintf(stderr, "memory allocation failure\n");
		exit(1);
		}
	hent->lcrit = lcrit;
	hent->lwarn = lwarn;
	hent->hwarn = hwarn;
	hent->hcrit = hcrit;
	}

void hinit() {
	hashadd("Disk_space", 3, 20, 110, 120);
	hashadd("Disk_inodes", 3, 20, 110, 120);
	hashadd("load1", -1, -1, 15, 30);
	hashadd("load5", -1, -1, 10, 25);
	hashadd("load15", -1, -1, 5, 20);
	hashadd("Uptime", -1, 3600, 42768000, 42949672);
	hashadd("Swap_free", 10, 50, 110, 120);
	hashadd("Processes", -1, -1, 150, 200);
	}
	
int thresher(char *name, long val) {
	struct hnode *hent = hasher(name, 0);
	int rc;
	if (hent) {
		if (val > hent->lwarn && val < hent->hwarn) {
			rc = 0;
			}
		else if (val > hent->lcrit && val < hent->hcrit) {
			rc = 1;
			}
		else {
			rc = 2;
			}
		}
	else {
		rc = 3;
		}
	return rc;
	}

int gthresher(char *name, long val) {
	int c = thresher(name, val);
	if (c == 3) c = 4;
	return c;
	}

int gnormalize(int c) {
	if (c & 4) {
		c = 3;
		}
	else if (c & 2) {
		c = 2;
		}
	return c;
	}

