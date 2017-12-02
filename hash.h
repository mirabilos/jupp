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
__IDSTRING(rcsid_hash_h, "$MirOS: contrib/code/jupp/hash.h,v 1.4 2017/12/02 17:00:48 tg Exp $");
#endif

unsigned long hash PARAMS((unsigned char *s));
HASH *htmk PARAMS((int len));
void htrm PARAMS((HASH *ht));
void *htadd PARAMS((HASH *ht, unsigned char *name, void *val));
void *htfind PARAMS((HASH *ht, unsigned char *name));

#endif
