/* $MirOS: contrib/code/jupp/regex.h,v 1.2 2008/05/13 13:08:24 tg Exp $ */
/*
 *	Regular expression subroutines
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#ifndef _JOE_REGEX_H
#define _JOE_REGEX_H 1

#include "config.h"
#include "types.h"

int escape PARAMS((int utf8,unsigned char **a, int *b));
int pmatch PARAMS((unsigned char **pieces, unsigned char *regex, int len, P *p, int n, int icase));

#endif
