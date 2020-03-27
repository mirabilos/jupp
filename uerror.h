/*
 *	Compiler error handler
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#ifndef JUPP_UERROR_H
#define JUPP_UERROR_H

#ifdef EXTERN_B_C
__IDSTRING(rcsid_uerror_h, "$MirOS: contrib/code/jupp/uerror.h,v 1.6 2020/03/27 06:38:59 tg Exp $");
#endif

int unxterr(BW *bw);
int uprverr(BW *bw);
int parserrb(B *b);
int uparserr(BW *bw);
void inserr(unsigned char *name, long int where, long int n, int bol);
void delerr(unsigned char *name, long int where, long int n);
void abrerr(unsigned char *name);
void saverr(unsigned char *name);

#endif
