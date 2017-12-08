/*
 *	Prompt windows
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#ifndef _JOE_PW_H
#define _JOE_PW_H 1

#ifdef EXTERN_CMD_C
__IDSTRING(rcsid_pw_h, "$MirOS: contrib/code/jupp/pw.h,v 1.10 2017/12/08 02:17:22 tg Exp $");
#endif

/* BW *wmkpw(BW *bw,char *prompt,int (*func)(),char *huh,int (*abrt)(),
	     int (*tab)(),void *object,int *notify);
 * Create a prompt window for the given window
 */
BW *wmkpw(W *w, const unsigned char *prompt, B **history, jpoly_int *func, const unsigned char *huh, jpoly_int *abrt, jpoly_int *tab, void *object, int *notify, struct charmap *map);

int ucmplt(BW *bw, int k);

/* Function for TAB completion */

unsigned char **regsub(unsigned char **z, int len, unsigned char *s);

void cmplt_ins(BW *bw,unsigned char *line);

int cmplt_abrt(BW *bw,int x, unsigned char *line);

int cmplt_rtn(MENU *m,int x,unsigned char *line);

int simple_cmplt(BW *bw,unsigned char **list);

extern WATOM watompw;

#endif
