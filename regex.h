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
__IDSTRING(rcsid_regex_h, "$MirOS: contrib/code/jupp/regex.h,v 1.6 2017/12/06 23:02:04 tg Exp $");
#endif

int escape(int, unsigned char **, int *);
int pmatch(unsigned char **pieces, unsigned char *regex, int len, P *p, int n, int icase);

#endif
