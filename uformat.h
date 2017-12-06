/*
 * 	User text formatting functions
 * 	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#ifndef _JOE_UFORMAT_H
#define _JOE_UFORMAT_H 1

#ifdef EXTERN_CMD_C
__IDSTRING(rcsid_uformat_h, "$MirOS: contrib/code/jupp/uformat.h,v 1.6 2017/12/06 23:17:36 tg Exp $");
#endif

int ucenter(BW *bw);
P *pbop(P *p);
P *peop(P *p);
int ubop(BW *bw);
int ueop(BW *bw);
void wrapword(P *p, long int indent, int french, unsigned char *indents);
int uformat(BW *bw);
int ufmtblk(BW *bw);

#endif
