/*
 *	Compiler error handler
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#ifndef _JOE_UERROR_H
#define _JOE_UERROR_H 1

#ifdef EXTERN_B_C
__IDSTRING(rcsid_uerror_h, "$MirOS: contrib/code/jupp/uerror.h,v 1.4 2017/12/02 17:00:51 tg Exp $");
#endif

int unxterr PARAMS((BW *bw));
int uprverr PARAMS((BW *bw));
int parserrb PARAMS((B *b));
int uparserr PARAMS((BW *bw));
void inserr PARAMS((unsigned char *name, long int where, long int n, int bol));
void delerr PARAMS((unsigned char *name, long int where, long int n));
void abrerr PARAMS((unsigned char *name));
void saverr PARAMS((unsigned char *name));

#endif
