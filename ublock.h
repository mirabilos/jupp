/*
 * 	Highlighted block functions
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#ifndef _JOE_UBLOCK_H
#define _JOE_UBLOCK_H 1

#ifdef EXTERN_CMD_C
__IDSTRING(rcsid_ublock_h, "$MirOS: contrib/code/jupp/ublock.h,v 1.5 2017/12/06 21:17:01 tg Exp $");
#endif

extern int square;
extern int lightoff;
extern P *markb, *markk;

void pinsrect(P *cur, B *tmp, long int width, int usetabs);
int ptabrect(P *org, long int height, long int right);
void pclrrect(P *org, long int height, long int right, int usetabs);
void pdelrect(P *org, long int height, long int right);
B *pextrect(P *org, long int height, long int right);
int markv(int r);
int umarkb(BW *bw);
int umarkk(BW *bw);
int uswap(BW *bw);
int umarkl(BW *bw);
int utomarkb(BW *bw);
int utomarkk(BW *bw);
int utomarkbk(BW *bw);
int ublkdel(BW *bw);
int upicokill(BW *bw);
int ublkmove(BW *bw);
int ublkcpy(BW *bw);
int dowrite(BW *bw, unsigned char *s, void *object, int *notify);
int doinsf(BW *bw, unsigned char *s, void *object, int *notify);
void setindent(BW *bw);
int urindent(BW *bw);
int ulindent(BW *bw);
int ufilt(BW *bw);
int unmark(BW *bw);
int udrop(BW *bw);
int utoggle_marking(BW *bw);
int ubegin_marking(BW *bw);
int uselect(BW *bw);
int upsh(BW *bw);
int upop(BW *bw);
int ulower(BW *bw);
int uupper(BW *bw);
extern int nstack;

#endif
