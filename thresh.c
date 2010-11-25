/* thresh.c - threshold processing
 *
 *	Copyright (C) 2006,2010 Anthony de Boer
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

#define HASHSIZE 1009

/*
 * threshold data is kept in a hash table so that we can look up each
 * discovered bit of information, see if there are thresholds, and set
 * severity appropriately.
 */

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

int showitems = 0;

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
	if (showitems) {
		printf("limit %s %ld %ld %ld %ld\n", s, lcrit, lwarn, hwarn, hcrit);
		}
	}

void hinit(int showdefaults) {
	showitems = showdefaults;
	hashadd("Disk_space", 3, 20, 110, 120);
	hashadd("Disk_inodes", 3, 20, 110, 120);
	hashadd("load1", -1, -1, 15, 30);
	hashadd("load5", -1, -1, 10, 25);
	hashadd("load15", -1, -1, 5, 20);
	hashadd("Uptime", -1, 3600, 42768000, 42949672);
	hashadd("Swap_free", 10, 50, 110, 120);
	hashadd("Processes", -1, -1, 150, 200);
	if (showdefaults) exit(0);
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

