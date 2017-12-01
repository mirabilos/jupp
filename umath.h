/* $MirOS: contrib/code/jupp/umath.h,v 1.6 2017/12/01 23:26:24 tg Exp $ */
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

#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif

extern volatile sig_atomic_t merrf;       
extern const unsigned char *merrt;

double calc(BW *bw, unsigned char *s);
int umath(BW *bw);
int umathins(BW *bw);
int umathres(BW *bw);

#define calcl(bw,s)	((long)calc((bw), (s)))
#define calcldec(bw,s)	((long)(calc((bw), (s)) - 1.0))

#endif
