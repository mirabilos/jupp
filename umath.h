/* $MirOS: contrib/code/jupp/umath.h,v 1.3 2012/12/30 17:10:58 tg Exp $ */
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

extern const unsigned char * volatile merr;
double calc(BW *bw, unsigned char *s);
int umath(BW *bw);

#endif
