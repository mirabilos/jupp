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
__IDSTRING(rcsid_uformat_h, "$MirOS: contrib/code/jupp/uformat.h,v 1.4 2017/12/02 17:00:51 tg Exp $");
#endif

int ucenter PARAMS((BW *bw));
P *pbop PARAMS((P *p));
P *peop PARAMS((P *p));
int ubop PARAMS((BW *bw));
int ueop PARAMS((BW *bw));
void wrapword PARAMS((P *p, long int indent, int french, unsigned char *indents));
int uformat PARAMS((BW *bw));
int ufmtblk PARAMS((BW *bw));

#endif
