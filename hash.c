/*
 *	Simple hash table
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#include "config.h"
#include "types.h"

#include <string.h>

#include "hash.h"
#include "utils.h"

#define hnext(accu, c) (((accu) << 4) + ((accu) >> 28) + (c))

static HENTRY *freentry = NULL;

unsigned long hash(unsigned char *s)
{
	unsigned long accu = 0;

	while (*s) {
		accu = hnext(accu, *s++);
	}
	return accu;
}

HASH *htmk(int len)
{
	HASH *t = (HASH *) joe_malloc(sizeof(HASH));

	t->len = len - 1;
	t->tab = (HENTRY **) joe_calloc(sizeof(HENTRY *), len);
	return t;
}

void htrm(HASH *ht)
{
	joe_free(ht->tab);
	joe_free(ht);
}

void *htadd(HASH *ht, unsigned char *name, void *val)
{
	int idx = hash(name) & ht->len;
	HENTRY *entry;
	int x;

	if (!freentry) {
		entry = (HENTRY *) joe_malloc(sizeof(HENTRY) *64);
		for (x = 0; x != 64; ++x) {
			entry[x].next = freentry;
			freentry = entry + x;
		}
	}
	entry = freentry;
	freentry = entry->next;
	entry->next = ht->tab[idx];
	ht->tab[idx] = entry;
	entry->name = name;
	return entry->val = val;
}

void *htfind(HASH *ht, unsigned char *name)
{
	HENTRY *e;

	for (e = ht->tab[hash(name) & ht->len]; e; e = e->next) {
		if (!strcmp(e->name, name)) {
			return e->val;
		}
	}
	return NULL;
}
