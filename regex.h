/*
 *	Regular expression subroutines
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#ifndef JUPP_REGEX_H
#define JUPP_REGEX_H

#ifdef EXTERN_RC_C
__IDSTRING(rcsid_regex_h, "$MirOS: contrib/code/jupp/regex.h,v 1.7 2020/03/27 06:38:58 tg Exp $");
#endif

int escape(int, unsigned char **, int *);
int pmatch(unsigned char **pieces, unsigned char *regex, int len, P *p, int n, int icase);

#endif
