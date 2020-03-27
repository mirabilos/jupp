/*
 * 	User text formatting functions
 * 	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#ifndef JUPP_UFORMAT_H
#define JUPP_UFORMAT_H

#ifdef EXTERN_CMD_C
__IDSTRING(rcsid_uformat_h, "$MirOS: contrib/code/jupp/uformat.h,v 1.7 2020/03/27 06:38:59 tg Exp $");
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
