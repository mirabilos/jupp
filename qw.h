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
__IDSTRING(rcsid_qw_h, "$MirOS: contrib/code/jupp/qw.h,v 1.4 2017/12/02 17:00:49 tg Exp $");
#endif

/* QW *mkqw(W *w, char *prompt, int (*func)(), int (*abrt)(), void *object);
 * Create a query window for the given window
 */
/* FIXME: ??? ----> */
QW *mkqw PARAMS((W *w, unsigned char *prompt, int len, int (*func) (/* ??? */), int (*abrt) (/* ??? */), void *object, int *notify));
QW *mkqwna PARAMS((W *w, unsigned char *prompt, int len, int (*func) (/* ??? */), int (*abrt) (/* ??? */), void *object, int *notify));
QW *mkqwnsr PARAMS((W *w, unsigned char *prompt, int len, int (*func) (/* ??? */), int (*abrt) (/* ??? */), void *object, int *notify));

#endif
