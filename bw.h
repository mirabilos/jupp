/*
 *	Edit buffer window generation
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#ifndef JUPP_BW_H
#define JUPP_BW_H

#ifdef EXTERN_CMD_C
__IDSTRING(rcsid_bw_h, "$MirOS: contrib/code/jupp/bw.h,v 1.9 2020/03/27 06:38:55 tg Exp $");
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
