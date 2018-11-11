/*
 * 	Doubly linked list primitives
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#include "config.h"
#include "types.h"

__RCSID("$MirOS: contrib/code/jupp/queue.c,v 1.6 2018/11/11 18:15:37 tg Exp $");

#include <stdlib.h>

#include "queue.h"
#include "utils.h"

void *QUEUE;
void *ITEM;
void *LAST;

void *
alitem(void *list, size_t itemsize)
{
	STDITEM	*freelist = (STDITEM *)list;

	if (qempty(STDITEM, link, freelist)) {
		size_t num = 16;
		unsigned char *i = malloc(itemsize * num);

		while (num--) {
			enquef(STDITEM, link, freelist, i);
			i += itemsize;
		}
	}
	return (void *)deque_f(STDITEM, link, freelist->link.prev);
}

void frchn(void *list, void *ch)
{
	STDITEM *freelist = (STDITEM *)list;
	STDITEM *chn = (STDITEM *)ch;
	STDITEM *i;

	if ((i = chn->link.prev) != chn) {
		deque(STDITEM, link, chn);
		splicef(STDITEM, link, freelist, i);
	}
}
