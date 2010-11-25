/* hashtab.c */

/* 
 *	Copyright (C) 2009 Anthony de Boer
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

#include <string.h>
#include "hashtab.h"

struct hashnode {
	struct hashnode* next;
	size_t payloadoff;
	char key[];
	};

struct hashtable {
	struct hashnode ** hashes;
	struct hashnode ** sorted;
	unsigned int hashsize;
	unsigned int entries;
	size_t payloadsize;
	};

HTAB hashcreate(size_t paysize, unsigned int tabsize)
{
	struct hashtable * ht = calloc(1, sizeof(struct hashtable));
	if (ht) {
		ht->hashes = calloc(tabsize, sizeof(struct hashtable*));
		if (ht->hashes) {
			ht->sorted = NULL;
			ht->hashsize = tabsize;
			ht->entries = 0;
			ht->payloadsize = paysize;
		} else {
			free(ht);
			ht = NULL;
		}
	}
	return ht;
}

static int hashit(const char *source)
{
	unsigned int x = 0;
	unsigned int c;
	while(c = *source++) {
		x = 3*x + c;
	}
	return x;
}

int hashfind(HTAB ht, const char *key, int create, void ** eptr)
{
	struct hashnode ** hpp;
	struct hashnode * hp;

	hpp = (ht->hashes) + (hashit(key) % ht->hashsize);

	while(hp = *hpp) {
		if (strcmp(key, hp->key) == 0) {
			*eptr = (void*)(((char*)hp)+hp->payloadoff);
			return 1;
		} else {
			hpp = &(hp->next);
		}
	}
	if (create) {
		size_t slen = strlen(key) + 1;
		size_t poff = (sizeof(struct hashnode) + slen + 7) & ~7;
		struct hashnode * nn = calloc(1, poff + ht->payloadsize);
		if (nn) {
			nn->next = NULL;
			nn->payloadoff = poff;
			memcpy(nn->key, key, slen);
			*hpp = nn;
			*eptr = (void*)(((char*)nn) + poff);
			ht->entries++;
			if (ht->sorted) {
				free(ht->sorted);
				ht->sorted = NULL;
			}
			return 0;
		}
	}
	*eptr = NULL;
	return 0;
}

void hashfree(HTAB ht)
{
	unsigned int i;
	for (i=0; i<ht->hashsize; i++) {
		struct hashnode * hp = ht->hashes[i];
		struct hashnode * np;
		while(hp) {
			np = hp->next;
			free(hp);
			hp = np;
		}
	}
	free (ht->hashes);
	if (ht->sorted) free (ht->sorted);
	free (ht);
}

void hashiterate(HTAB ht, hiterator * hdo, void * userdata)
{
	unsigned int i;
	for (i=0; i<ht->hashsize; i++) {
		struct hashnode * hp = ht->hashes[i];
		while(hp) {
			void * vp = (void*)(((char*)hp)+hp->payloadoff);
			hdo(hp->key, vp, userdata);
			hp = hp->next;
		}
	}
}

static int hashqcmp(const void *a, const void *b)
{
	return strcmp(
		(*(const struct hashnode **)a)->key,
		(*(const struct hashnode **)b)->key);
}

int hashsort(HTAB ht, hiterator * hdo, void * userdata)
{
	unsigned int i;
	if (ht->sorted == NULL) {
		struct hashnode ** sl;
		unsigned int chk = 0;
		sl = calloc(ht->entries, sizeof(struct hashnode *));
		if (sl == NULL) return -1;
		ht->sorted = sl;
		for (i=0; i<ht->hashsize; i++) {
			struct hashnode * hp = ht->hashes[i];
			while(hp) {
				*sl++ = hp;
				hp = hp->next;
				chk++;
			}
		}
		qsort(ht->sorted, ht->entries, sizeof(struct hashnode *), hashqcmp);
	}
	for(i=0; i<ht->entries; i++) {
		struct hashnode *hp = ht->sorted[i];
		void * vp = (void*)(((char*)hp)+hp->payloadoff);
		hdo(hp->key, vp, userdata);
	}
	return 0;
}

