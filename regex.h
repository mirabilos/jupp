/*
 *	Regular expression subroutines
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#ifndef _JOE_REGEX_H
#define _JOE_REGEX_H 1

#ifdef EXTERN_RC_C
__IDSTRING(rcsid_regex_h, "$MirOS: contrib/code/jupp/regex.h,v 1.4 2017/12/02 17:00:50 tg Exp $");
#endif

int escape PARAMS((int utf8,unsigned char **a, int *b));
int pmatch PARAMS((unsigned char **pieces, unsigned char *regex, int len, P *p, int n, int icase));

#endif
