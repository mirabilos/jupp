/*
 *	Keyboard macros
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#ifndef JUPP_MACRO_H
#define JUPP_MACRO_H

#ifdef EXTERN
__IDSTRING(rcsid_macro_h, "$MirOS: contrib/code/jupp/macro.h,v 1.6 2020/03/27 06:38:57 tg Exp $");
#endif

/* Set when macro is recording: for status line */
extern struct recmac *recmac;

/* Macro construction functions */
MACRO *mkmacro(int k, int arg, int n, CMD *cmd);
void addmacro(MACRO *macro, MACRO *m);
MACRO *dupmacro(MACRO *mac);
void rmmacro(MACRO *macro);
MACRO *macstk(MACRO *m, int k);
MACRO *macsta(MACRO *m, int a);

void chmac(void);

/* Text to macro / Macro to text */
MACRO *mparse(MACRO *m, unsigned char *buf, int *sta);
unsigned char *mtext(unsigned char *s, MACRO *m);

/* Execute a macro */
extern MACRO *curmacro;
int exemac(MACRO *m);
int exmacro(MACRO *m, int u);

/* Keyboard macros user interface */
int uplay(BW *bw, int c);
int ustop(void);
int urecord(BW *bw, int c);
int uquery(BW *bw);
int umacros(BW *bw);

/* Repeat prefix user command */
int uarg(BW *bw);
int uuarg(BW *bw, int c);

#endif
