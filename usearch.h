/*
 *	Search & Replace system
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#ifndef _JOE_USEARCH_H
#define _JOE_USEARCH_H 1

#ifdef EXTERN_CMD_C
__RCSID("$MirOS: contrib/code/jupp/usearch.h,v 1.3 2017/12/02 02:07:36 tg Exp $");
#endif

SRCH *mksrch PARAMS((unsigned char *pattern, unsigned char *replacement, int ignore, int backwards, int repeat, int replace, int rest));
void rmsrch PARAMS((SRCH *srch));

int dopfnext PARAMS((BW *bw, SRCH *srch, int *notify));

int pffirst PARAMS((BW *bw));
int pfnext PARAMS((BW *bw));

int pqrepl PARAMS((BW *bw));
int prfirst PARAMS((BW *bw));

int ufinish PARAMS((BW *bw));

#endif
