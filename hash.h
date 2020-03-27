/*
 *	Simple hash table
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#ifndef JUPP_HASH_H
#define JUPP_HASH_H

#ifdef EXTERN_CMD_C
__IDSTRING(rcsid_hash_h, "$MirOS: contrib/code/jupp/hash.h,v 1.7 2020/03/27 06:38:56 tg Exp $");
#endif

unsigned long hash(const unsigned char *s);
HASH *htmk(int len);
void htrm(HASH *ht);
void *htadd(HASH *ht, const unsigned char *name, void *val);
void *htfind(HASH *ht, const unsigned char *name);

#endif
