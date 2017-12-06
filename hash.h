/*
 *	Simple hash table
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#ifndef _JOE_HASH_H
#define _JOE_HASH_H 1

#ifdef EXTERN_CMD_C
__IDSTRING(rcsid_hash_h, "$MirOS: contrib/code/jupp/hash.h,v 1.5 2017/12/06 21:16:56 tg Exp $");
#endif

unsigned long hash(unsigned char *s);
HASH *htmk(int len);
void htrm(HASH *ht);
void *htadd(HASH *ht, unsigned char *name, void *val);
void *htfind(HASH *ht, unsigned char *name);

#endif
