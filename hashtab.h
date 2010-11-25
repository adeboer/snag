/* hashtab.h */

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

#include <stdlib.h>

struct hashtable;
typedef struct hashtable * HTAB;

/*
 * Create a hash table.
 * paysize - sizeof(struct xyz) stored at each node
 * tabsize - hash size, ideally prime and a bit bigger than expected #nodes
 *
 * NB: keys don't need to be stored inside node
 * returns - HTAB handle used in subsequent calls; handle will evaluate
 * true in if() on success, false on failure.
 */
HTAB hashcreate(size_t paysize, unsigned int tabsize);

/*
 * ht - handle to hashtable to search
 * key - key string of entry to search for
 * create - if true, create if not found (new struct will be zeroed)
 * eptr - pointer will be updated to point to found/created struct
 * returns 1 if found, 0 if not found or created
 */
int hashfind(HTAB ht, const char *key, int create, void ** eptr);

/*
 * template for user-supplied iterator function, called at each node
 * with key, node struct pointer, and a pointer to user-supplied data
 */
typedef void hiterator(const char *, void *, void *);

/*
 * call iterator function on each entry, in hashed order
 */
void hashiterate(HTAB ht, hiterator * hdo, void * userdata);

/*
 * call iterator function on each entry, in sorted order
 * returns 0 on success, -1 if unable to sort
 */
int hashsort(HTAB ht, hiterator * hdo, void * userdata);

/*
 * free a hashtable and all associated allocations
 */
void hashfree(HTAB ht);

/* end of definitions */

