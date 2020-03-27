/*
 * 	Shell-window functions
 *	Copyright (C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#ifndef JUPP_USHELL_H
#define JUPP_USHELL_H

#ifdef EXTERN_CMD_C
__IDSTRING(rcsid_ushell_h, "$MirOS: contrib/code/jupp/ushell.h,v 1.7 2020/03/27 06:39:00 tg Exp $");
#endif

int ubknd(BW *bw);
int ukillpid(BW *bw);
int urun(BW *bw);
int ubuild(BW *bw);

const void *getushell(void);

#endif
