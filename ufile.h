/*
 * 	User file operations
 *	Copyright
 *	(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#ifndef JUPP_UFILE_H
#define JUPP_UFILE_H

#ifdef EXTERN_CMD_C
__IDSTRING(rcsid_ufile_h, "$MirOS: contrib/code/jupp/ufile.h,v 1.9 2020/03/27 06:38:59 tg Exp $");
#endif

extern int exask;

void genexmsg(BW *bw, int saved, unsigned char *name);

int ublksave(BW *bw);
int ushell(BW *bw);
int usave(BW *bw);
int uedit(BW *bw);
int uswitch(BW *bw);
int uscratch(BW *bw);
int uinsf(BW *bw);
int uexsve(BW *bw);
int unbuf(BW *bw);
int upbuf(BW *bw);
int uask(BW *bw);
int ubufed(BW *bw);
int ulose(BW *bw);
int okrepl(BW *bw);
int doswitch(BW *bw, unsigned char *s, void *obj, int *notify);
int uquerysave(BW *bw);
int ukilljoe(BW *bw);
int uabendjoe(BW *bw);
int usync(BW *bw);

#endif
