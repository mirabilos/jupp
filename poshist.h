/*
 *	Position history
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#ifndef _JOE_POSHIST_H
#define _JOE_POSHIST_H 1

#ifdef EXTERN_CMD_C
__IDSTRING(rcsid_poshist_h, "$MirOS: contrib/code/jupp/poshist.h,v 1.4 2017/12/02 17:00:49 tg Exp $");
#endif

void afterpos PARAMS((void));
void aftermove PARAMS((W *w, P *p));
void windie PARAMS((W *w));
int uprevpos PARAMS((BW *bw));
int unextpos PARAMS((BW *bw));

#endif
