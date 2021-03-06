/*
 *	Text editing windows
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#ifndef JUPP_TW_H
#define JUPP_TW_H

#ifdef EXTERN
__IDSTRING(rcsid_tw_h, "$MirOS: contrib/code/jupp/tw.h,v 1.7 2020/03/27 06:38:58 tg Exp $");
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
