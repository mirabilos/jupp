/*
 *	*rc file parser
 *	Copyright
 *		(C) 1992 Joseph H. Allen;
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#ifndef _JOE_RC_H
#define _JOE_RC_H 1

#ifdef EXTERN
__IDSTRING(rcsid_rc_h, "$MirOS: contrib/code/jupp/rc.h,v 1.12 2018/01/08 00:48:07 tg Exp $");
#endif

extern OPTIONS pdefault;
extern OPTIONS fdefault;
void setopt(B *b, const unsigned char *name);

/* KMAP *kmap_getcontext(char *name);
 * Find and return the KMAP for a given context name.  If none is found, an
 * empty kmap is created, bound to the context name, and returned.
 */
KMAP *kmap_getcontext(const unsigned char *name, int docreate);

/* int procrc(CAP *cap, char *name);  Process an rc file
   Returns 0 for success
	  -1 for file not found
	   1 for syntax error (errors written to stderr)
*/
int procrc(CAP *cap, const unsigned char *name);

int glopt(unsigned char *s, unsigned char *arg, OPTIONS *options, int set);

int umode(BW *bw);

void lazy_opts(OPTIONS *o);

#endif
