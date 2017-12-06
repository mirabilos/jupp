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
__IDSTRING(rcsid_umath_h, "$MirOS: contrib/code/jupp/umath.h,v 1.11 2017/12/06 21:17:03 tg Exp $");
#endif

#include <signal.h>

extern volatile sig_atomic_t merrf;
extern const unsigned char *merrt;

long calcl(BW *bw, unsigned char *s);

#if WANT_MATH
int umath(BW *bw);
int umathins(BW *bw);
int umathres(BW *bw);
#else
int unomath(BW *bw);

#define umath		unomath
#define umathins	unomath
#define umathres	unomath
#endif

#endif
