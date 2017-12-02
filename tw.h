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
__RCSID("$MirOS: contrib/code/jupp/tw.h,v 1.4 2017/12/02 02:07:33 tg Exp $");
#endif

BW *wmktw PARAMS((SCREEN *t, B *b));

int usplitw PARAMS((BW *bw));
int uduptw PARAMS((BW *bw));
int utw0 PARAMS((BASE *b));
int utw1 PARAMS((BASE *b));
int uabortbuf PARAMS((BW *bw));
int ucancel PARAMS((BW *bw, int k));
int uabort PARAMS((BW *bw, int k));
int uabort1 PARAMS((BW *bw, int k));
void setline PARAMS((B *b, long int line));
int abortit PARAMS((BW *bw));
extern int staen;

extern WATOM watomtw;

#endif
