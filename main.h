/*
 *	Editor startup and edit loop
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#ifndef JUPP_MAIN_H
#define JUPP_MAIN_H

#ifdef EXTERN_B_C
__IDSTRING(rcsid_main_h, "$MirOS: contrib/code/jupp/main.h,v 1.7 2020/03/27 06:38:57 tg Exp $");
#endif

extern const char null[];

extern unsigned char *exmsg;	/* Exit message */
extern int help;		/* Set to start with help on */
extern SCREEN *maint;		/* Primary screen */
void nungetc(int c);
void dofollows(void);
int edloop(int flg);
void edupd(int flg);

#endif
