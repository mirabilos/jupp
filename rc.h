/*
 *	*rc file parser
 *	Copyright
 *		(C) 1992 Joseph H. Allen; 
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#ifndef _JOE_RC_H
#define _JOE_RC_H 1

#include "config.h"
#include "types.h"

extern OPTIONS pdefault;
extern OPTIONS fdefault;
void setopt PARAMS((B *b, unsigned char *name));

/* KMAP *kmap_getcontext(char *name);
 * Find and return the KMAP for a given context name.  If none is found, an
 * empty kmap is created, bound to the context name, and returned.
 */
KMAP *kmap_getcontext PARAMS((unsigned char *name));

/* int procrc(CAP *cap, char *name);  Process an rc file
   Returns 0 for success
          -1 for file not found
           1 for syntax error (errors written to stderr)
*/
int procrc PARAMS((CAP *cap, unsigned char *name));

int glopt PARAMS((unsigned char *s, unsigned char *arg, OPTIONS *options, int set));

int umode PARAMS((BW *bw));

#endif
