/* $MirOS: contrib/code/jupp/umath.h,v 1.2 2008/05/13 13:08:28 tg Exp $ */
/*
 *	Math
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#ifndef _JOE_UMATH_H
#define _JOE_UMATH_H 1

#include "config.h"
#include "types.h"

extern unsigned char *merr;
double calc(BW *bw, unsigned char *s);
int umath(BW *bw);

#endif
