/*
 *	Single-key query windows
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#ifndef JUPP_QW_H
#define JUPP_QW_H

#ifdef EXTERN_UFILE_C
__IDSTRING(rcsid_qw_h, "$MirOS: contrib/code/jupp/qw.h,v 1.8 2020/03/27 06:38:57 tg Exp $");
#endif

/* QW *mkqw(W *w, char *prompt, int (*func)(), int (*abrt)(), void *object);
 * Create a query window for the given window
 */
/* FIXME: ??? ----> */
QW *mkqw(W *w, const unsigned char *prompt, int len, jpoly_int *func, jpoly_int *abrt, void *object, int *notify);
QW *mkqwna(W *w, const unsigned char *prompt, int len, jpoly_int *func, jpoly_int *abrt, void *object, int *notify);
QW *mkqwnsr(W *w, const unsigned char *prompt, int len, jpoly_int *func, jpoly_int *abrt, void *object, int *notify);

#endif
