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
#include "hashtab.h"

#define HASHSIZE 1009

/*
 * threshold data is kept in a hash table so that we can look up each
 * discovered bit of information, see if there are thresholds, and set
 * severity appropriately.
 */

HTAB tparams;

struct hnode {
	long lcrit, lwarn, hwarn, hcrit;
	};

HTAB proctab;

struct pnode {
	int min, max, conf, running;
	};

HTAB vartab;

struct vnode {
	int value;
	};

void hashadd(char *s, long lcrit, long lwarn, long hwarn, long hcrit)
{
	struct hnode *hent;
	hashfind(tparams, s, 1, (void **)&hent);
	if (!hent) {
		fprintf(stderr, "memory allocation failure\n");
		exit(1);
		}
	hent->lcrit = lcrit;
	hent->lwarn = lwarn;
	hent->hwarn = hwarn;
	hent->hcrit = hcrit;
	if (showdefaults) {
		printf("limit %s %ld %ld %ld %ld\n", s, lcrit, lwarn, hwarn, hcrit);
	}
}

void hinit() {
	tparams = hashcreate(sizeof(struct hnode), HASHSIZE);
	proctab = hashcreate(sizeof(struct pnode), HASHSIZE);
	vartab = hashcreate(sizeof(struct vnode), HASHSIZE);
	hashadd("Disk_space", 3, 20, 110, 120);
	hashadd("Disk_inodes", 3, 20, 110, 120);
	hashadd("load1", -1, -1, 15, 30);
	hashadd("load5", -1, -1, 10, 25);
	hashadd("load15", -1, -1, 5, 20);
	hashadd("Uptime", -1, 3600, 42768000, 42949672);
	hashadd("Swap_free", 10, 50, 110, 120);
	hashadd("Processes", -1, -1, 150, 200);
	setvar("sysuidmax", 999);
	setvar("transient_time", 1000);
	setvar("max_same_proc", 10);
	if (showdefaults) exit(0);
	}
	
int thresher(char *name, long val) {
	struct hnode *hent;
	int rc;
	if (hashfind(tparams, name, 0, (void **)&hent)) {
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

void procadd(char *s, int min, int max)
{
	struct pnode *pent;
	hashfind(proctab, s, 1, (void **)&pent);
	if (!pent) {
		fprintf(stderr, "memory allocation failure\n");
		exit(1);
	}
	pent->min = min;
	pent->max = max;
	pent->conf = 1;
}

void procfound(char *s, int awhile)
{
	struct pnode *pent;
	hashfind(proctab, s, 1, (void **)&pent);
	if (!pent) {
		fprintf(stderr, "memory allocation failure\n");
		exit(1);
	}
	if (pent->conf == 0) {
		pent->max = -1;
		if (awhile) {
			pent->conf = 2;
		}
	}
	pent->running++;
}

struct procbuf {
	char *bptr;
	int bremain;
	int status;
	};

void prociter(const char *name, void *vpent, void *foo)
{
	struct pnode *pent = vpent;
	struct procbuf *pb = foo;
	/*printf("PROC %s : %d %d %d %d\n", name, pent->min, pent->max, pent->conf, pent->running);*/
	int realmax = pent->max;
	if (realmax < 0) realmax = getvar("max_same_proc");
	if (pent->running < pent->min || pent->running > realmax || pent->conf == 2) {
		if (pent->running == 0) {
			pb->status = 2;
		} else if (pb->status == 0) {
			pb->status = 1;
		}
		int rb = snprintf(pb->bptr, pb->bremain, " %s:%d", name, pent->running);
		if (rb < pb->bremain) {
			pb->bremain -= rb;
			pb->bptr += rb;
		} else {
			*(pb->bptr) = '\0';
		}
	}
}

void procfinal()
{
	char buff[400];
	struct procbuf pstr;
	buff[0] = '\0';
	pstr.bptr = buff;
	pstr.bremain = sizeof(buff);
	pstr.status = 0;
	hashsort(proctab, prociter, &pstr);
	char *prx = pstr.status ? "Running" : "All OK";
	printf("Process list;%d;%s%s\n", pstr.status, prx, buff);
}

void setvar(char *s, int val)
{
	struct vnode *vent;
	hashfind(vartab, s, 1, (void **)&vent);
	if (!vent) {
		fprintf(stderr, "memory allocation failure\n");
		exit(1);
	}
	vent->value = val;
	if (showdefaults) {
		printf("%s = %d\n", s, val);
	}
}

int getvar(const char *name)
{
	struct vnode *vent;
	if (hashfind(vartab, name, 0, (void **)&vent)) {
		return vent->value;
	}
	return 0;
}
