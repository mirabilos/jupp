/*
 *	Simple hash table
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#include "config.h"
#include "types.h"

__RCSID("$MirOS: contrib/code/jupp/hash.c,v 1.7 2018/01/07 23:51:34 tg Exp $");

#include <stdlib.h>
#include <string.h>

#include "hash.h"
#include "utils.h"

#define hnext(accu, c) (((accu) << 4) + ((accu) >> 28) + (c))

static HENTRY *freentry = NULL;

unsigned long hash(const unsigned char *s)
{
	unsigned long accu = 0;

	while (*s) {
		accu = hnext(accu, *s++);
	}
	return accu;
}

HASH *htmk(int len)
{
	HASH *t = malloc(sizeof(HASH));

	t->len = len - 1;
	t->tab = calloc(len, sizeof(HENTRY *));
	return t;
}

void htrm(HASH *ht)
{
	free(ht->tab);
	free(ht);
}

void *htadd(HASH *ht, const unsigned char *name, void *val)
{
	int idx = hash(name) & ht->len;
	HENTRY *entry;
	int x;

	if (!freentry) {
		entry = ralloc(64, sizeof(HENTRY));
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

void *htfind(HASH *ht, const unsigned char *name)
{
	HENTRY *e;

	for (e = ht->tab[hash(name) & ht->len]; e; e = e->next) {
		if (!strcmp(e->name, name)) {
			return e->val;
		}
	}
	return NULL;
}
