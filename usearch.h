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
__IDSTRING(rcsid_usearch_h, "$MirOS: contrib/code/jupp/usearch.h,v 1.5 2017/12/06 21:17:03 tg Exp $");
#endif

SRCH *mksrch(unsigned char *pattern, unsigned char *replacement, int ignore, int backwards, int repeat, int replace, int rest);
void rmsrch(SRCH *srch);

int dopfnext(BW *bw, SRCH *srch, int *notify);

int pffirst(BW *bw);
int pfnext(BW *bw);

int pqrepl(BW *bw);
int prfirst(BW *bw);

int ufinish(BW *bw);

#endif
