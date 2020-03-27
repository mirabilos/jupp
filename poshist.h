/*
 *	Position history
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#ifndef JUPP_POSHIST_H
#define JUPP_POSHIST_H

#ifdef EXTERN_CMD_C
__IDSTRING(rcsid_poshist_h, "$MirOS: contrib/code/jupp/poshist.h,v 1.6 2020/03/27 06:38:57 tg Exp $");
#endif

void afterpos(void);
void aftermove(W *w, P *p);
void windie(W *w);
int uprevpos(BW *bw);
int unextpos(BW *bw);

#endif
