/*
 *	Math
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#ifndef _JOE_UMATH_H
#define _JOE_UMATH_H 1

#ifdef EXTERN_CMD_C
__RCSID("$MirOS: contrib/code/jupp/umath.h,v 1.8 2017/12/02 04:15:29 tg Exp $");
#endif

#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif

extern volatile sig_atomic_t merrf;       
extern const unsigned char *merrt;

#if WANT_MATH
double calc(BW *bw, unsigned char *s);
int umath(BW *bw);
int umathins(BW *bw);
int umathres(BW *bw);

#define calcl(bw,s)	((long)calc((bw), (s)))
#define calcldec(bw,s)	((long)(calc((bw), (s)) - 1.0))

#else

long calcl(BW *bw, unsigned char *s);
int unomath(BW *bw);

#define umath		unomath
#define umathins	unomath
#define umathres	unomath
#define calcldec(bw,s)	(calcl((bw), (s)) - 1L)

#endif /* !WANT_MATH */

#endif
