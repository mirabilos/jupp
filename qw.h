/*
 *	Single-key query windows
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#ifndef _JOE_QW_H
#define _JOE_QW_H 1

#ifdef EXTERN_UFILE_C
__RCSID("$MirOS: contrib/code/jupp/qw.h,v 1.3 2017/12/02 02:07:30 tg Exp $");
#endif

/* QW *mkqw(W *w, char *prompt, int (*func)(), int (*abrt)(), void *object);
 * Create a query window for the given window
 */
/* FIXME: ??? ----> */
QW *mkqw PARAMS((W *w, unsigned char *prompt, int len, int (*func) (/* ??? */), int (*abrt) (/* ??? */), void *object, int *notify));
QW *mkqwna PARAMS((W *w, unsigned char *prompt, int len, int (*func) (/* ??? */), int (*abrt) (/* ??? */), void *object, int *notify));
QW *mkqwnsr PARAMS((W *w, unsigned char *prompt, int len, int (*func) (/* ??? */), int (*abrt) (/* ??? */), void *object, int *notify));

#endif
