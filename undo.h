/*
 *	UNDO system
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#ifndef _JOE_UNDO_H
#define _JOE_UNDO_H 1

#ifdef EXTERN_B_C
__IDSTRING(rcsid_undo_h, "$MirOS: contrib/code/jupp/undo.h,v 1.5 2017/12/06 21:17:03 tg Exp $");
#endif

extern int inundo;
extern int justkilled;

UNDO *undomk(B *b);
void undorm(UNDO *undo);
int uundo(BW *bw);
int uredo(BW *bw);
void umclear(void);
void undomark(void);
void undoins(UNDO *undo, P *p, long int size);
void undodel(UNDO *undo, long int where, B *b);
int uyank(BW *bw);
int uyankpop(BW *bw);
int uyapp(BW *bw);
int unotmod(BW *bw);
int ucopy(BW *bw);

#endif
