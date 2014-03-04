/* $MirOS: contrib/code/jupp/bw.h,v 1.4 2014/03/04 23:04:11 tg Exp $ */
/*
 *	Edit buffer window generation
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#ifndef _JOE_BW_H
#define _JOE_BW_H 1

#include "config.h"
#include "types.h"

extern int dspasis;
extern int mid;

void bwfllw PARAMS((BW *w));
void bwins PARAMS((BW *w, long int l, long int n, int flg));
void bwdel PARAMS((BW *w, long int l, long int n, int flg));
void bwgen PARAMS((BW *w, int linums));
void bwgenh PARAMS((BW *w));
BW *bwmk PARAMS((W *window, B *b, int prompt));
void bwmove PARAMS((BW *w, int x, int y));
void bwresz PARAMS((BW *w, int wi, int he));
void bwrm PARAMS((BW *w));
#undef ustat
#define ustat ustat_j /* to avoid Linux libc4 conflict */
int ustat PARAMS((BW *bw));
int ucrawll PARAMS((BW *bw));
int ucrawlr PARAMS((BW *bw));
void orphit PARAMS((BW *bw));

#endif
