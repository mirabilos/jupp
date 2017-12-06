/*
 *	Editor startup and edit loop
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#ifndef _JOE_MAIN_H
#define _JOE_MAIN_H 1

#ifdef EXTERN_B_C
__IDSTRING(rcsid_main_h, "$MirOS: contrib/code/jupp/main.h,v 1.6 2017/12/06 21:16:58 tg Exp $");
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
