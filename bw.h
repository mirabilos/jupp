/*
 *	Edit buffer window generation
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#ifndef _JOE_BW_H
#define _JOE_BW_H 1

#ifdef EXTERN_CMD_C
__IDSTRING(rcsid_bw_h, "$MirOS: contrib/code/jupp/bw.h,v 1.8 2017/12/07 02:10:16 tg Exp $");
#endif

extern int dspasis;
extern int mid;

void bwfllw(jobject);
void bwins(BW *w, long int l, long int n, int flg);
void bwdel(BW *w, long int l, long int n, int flg);
void bwgen(BW *w, int linums);
void bwgenh(BW *w);
BW *bwmk(W *window, B *b, int prompt);
void bwmove(BW *w, int x, int y);
void bwresz(BW *w, int wi, int he);
void bwrm(BW *w);
int ustat_j(BW *bw);
int ucrawll(BW *bw);
int ucrawlr(BW *bw);
void orphit(BW *bw);

#endif
