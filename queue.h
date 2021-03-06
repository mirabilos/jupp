/*
 *	Doubly linked list primitives
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#ifndef JUPP_QUEUE_H
#define JUPP_QUEUE_H

#ifdef EXTERN_B_C
__IDSTRING(rcsid_queue_h, "$MirOS: contrib/code/jupp/queue.h,v 1.9 2020/10/30 03:11:05 tg Exp $");
#endif

extern void *ITEM;
extern void *QUEUE;
extern void *LAST;

#define izque(type,member,item) do { \
	QUEUE = (void *)(item); \
	((type *)QUEUE)->member.prev = (type *)QUEUE; \
	((type *)QUEUE)->member.next = (type *)QUEUE; \
	} while(0)

#define deque(type,member,item) do { \
	ITEM = (void *)(item); \
	((type *)ITEM)->member.prev->member.next = ((type *)ITEM)->member.next; \
	((type *)ITEM)->member.next->member.prev = ((type *)ITEM)->member.prev; \
	} while(0)

#define deque_f(type,member,item) \
	( \
	ITEM=(void *)(item), \
	((type *)ITEM)->member.prev->member.next=((type *)ITEM)->member.next, \
	((type *)ITEM)->member.next->member.prev=((type *)ITEM)->member.prev, \
	(type *)ITEM \
	)

#define qempty(type,member,item) \
	( \
	QUEUE=(void *)(item), \
	(type *)QUEUE==((type *)QUEUE)->member.next \
	)

#define enquef(type,member,queue,item) do { \
	ITEM = (void *)(item); \
	QUEUE = (void *)(queue); \
	((type *)ITEM)->member.next = ((type *)QUEUE)->member.next; \
	((type *)ITEM)->member.prev = (type *)QUEUE; \
	((type *)QUEUE)->member.next->member.prev = (type *)ITEM; \
	((type *)QUEUE)->member.next = (type *)ITEM; \
	} while(0)

#define enqueb(type,member,queue,item) do { \
	ITEM = (void *)(item); \
	QUEUE = (void *)(queue); \
	((type *)ITEM)->member.next = (type *)QUEUE; \
	((type *)ITEM)->member.prev = ((type *)QUEUE)->member.prev; \
	((type *)QUEUE)->member.prev->member.next = (type *)ITEM; \
	((type *)QUEUE)->member.prev = (type *)ITEM; \
	} while(0)

#define enqueb_f(type,member,queue,item) \
	( \
	ITEM=(void *)(item), \
	QUEUE=(void *)(queue), \
	((type *)ITEM)->member.next=(type *)QUEUE, \
	((type *)ITEM)->member.prev=((type *)QUEUE)->member.prev, \
	((type *)QUEUE)->member.prev->member.next=(type *)ITEM, \
	((type *)QUEUE)->member.prev=(type *)ITEM, \
	(type *)ITEM \
	)

#define promote(type,member,queue,item) do { \
	LAST = (void *)deque_f(type, member, (item)); \
	enquef(type, member, (queue), LAST); \
	} while (/* CONSTCOND */ 0)

#define demote(type,member,queue,item) do { \
	LAST = (void *)deque_f(type, member, (item)); \
	enqueb(type, member, (queue), LAST); \
	} while (/* CONSTCOND */ 0)

#define splicef(type,member,queue,chain) do { \
	ITEM = (void *)(chain); \
	LAST = (void *)((type *)ITEM)->member.prev; \
	QUEUE = (void *)(queue); \
	((type *)LAST)->member.next = ((type *)QUEUE)->member.next; \
	((type *)ITEM)->member.prev = (type *)QUEUE; \
	((type *)QUEUE)->member.next->member.prev = (type *)LAST; \
	((type *)QUEUE)->member.next = (type *)ITEM; \
	} while(0)

#define spliceb(type,member,queue,chain) do { \
	ITEM = (void *)(chain); \
	LAST = (void *)((type *)ITEM)->member.prev; \
	QUEUE = (void *)(queue); \
	((type *)LAST)->member.next = (type *)QUEUE; \
	((type *)ITEM)->member.prev = ((type *)QUEUE)->member.prev; \
	((type *)QUEUE)->member.prev->member.next = (type *)ITEM; \
	((type *)QUEUE)->member.prev = (type *)LAST; \
	} while(0)

#define spliceb_f(type,member,queue,chain) \
	( \
	ITEM=(void *)(chain), \
	LAST=(void *)((type *)ITEM)->member.prev, \
	QUEUE=(void *)(queue), \
	((type *)LAST)->member.next=(type *)QUEUE, \
	((type *)ITEM)->member.prev=((type *)QUEUE)->member.prev, \
	((type *)QUEUE)->member.prev->member.next=(type *)ITEM, \
	((type *)QUEUE)->member.prev=(type *)LAST, \
	(type *)ITEM \
	)

#define snip(type,member,first,last) \
	( \
	ITEM=(void *)(first), \
	LAST=(void *)(last), \
	((type *)LAST)->member.next->member.prev=((type *)ITEM)->member.prev, \
	((type *)ITEM)->member.prev->member.next=((type *)LAST)->member.next, \
	((type *)ITEM)->member.prev=(type *)LAST, \
	((type *)LAST)->member.next=(type *)ITEM, \
	(type *)ITEM \
	)

void *alitem(void *list, size_t itemsize);
void frchn(void *list, void *ch);

#endif
