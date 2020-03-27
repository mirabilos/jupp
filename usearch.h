/*
 *	Search & Replace system
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#ifndef JUPP_USEARCH_H
#define JUPP_USEARCH_H

#ifdef EXTERN_CMD_C
__IDSTRING(rcsid_usearch_h, "$MirOS: contrib/code/jupp/usearch.h,v 1.6 2020/03/27 06:39:00 tg Exp $");
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
