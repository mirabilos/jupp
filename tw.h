/*
 *	Text editing windows
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#ifndef _JOE_TW_H
#define _JOE_TW_H 1

#ifdef EXTERN
__IDSTRING(rcsid_tw_h, "$MirOS: contrib/code/jupp/tw.h,v 1.6 2017/12/06 21:17:01 tg Exp $");
#endif

BW *wmktw(SCREEN *t, B *b);

int usplitw(BW *bw);
int uduptw(BW *bw);
int utw0(BASE *b);
int utw1(BASE *b);
int uabortbuf(BW *bw);
int ucancel(BW *bw, int k);
int uabort(BW *bw, int k);
int uabort1(BW *bw, int k);
void setline(B *b, long int line);
int abortit(BW *bw);
extern int staen;

extern WATOM watomtw;

#endif
