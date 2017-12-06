/*
 * 	Shell-window functions
 *	Copyright (C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#ifndef _JOE_USHELL_H
#define _JOE_USHELL_H 1

#ifdef EXTERN_CMD_C
__IDSTRING(rcsid_ushell_h, "$MirOS: contrib/code/jupp/ushell.h,v 1.6 2017/12/06 21:17:03 tg Exp $");
#endif

int ubknd(BW *bw);
int ukillpid(BW *bw);
int urun(BW *bw);
int ubuild(BW *bw);

const void *getushell(void);

#endif
