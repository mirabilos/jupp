/* $MirOS: contrib/code/jupp/main.h,v 1.3 2012/12/22 00:06:12 tg Exp $ */
/*
 *	Editor startup and edit loop
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#ifndef _JOE_MAIN_H
#define _JOE_MAIN_H 1

#include "config.h"
#include "types.h"

extern const char null[];

extern unsigned char *exmsg;	/* Exit message */
extern int help;		/* Set to start with help on */
extern SCREEN *maint;		/* Primary screen */
void nungetc PARAMS((int c));
void dofollows PARAMS((void));
int edloop PARAMS((int flg));
void edupd PARAMS((int flg));

#endif
