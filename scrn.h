/*
 *	device-independent TTY interface for JOE
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#ifndef _JOE_SCRN_H
#define _JOE_SCRN_H 1

#ifdef EXTERN
__IDSTRING(rcsid_scrn_h, "$MirOS: contrib/code/jupp/scrn.h,v 1.12 2018/01/08 00:56:27 tg Exp $");
#endif

#include "tty.h"		/* ttputc() */

extern int skiptop;

/* SCRN *nopen(void);
 *
 * Open the screen (sets TTY mode so that screen may be used immediatly after
 * the 'nopen').
 */
SCRN *nopen(CAP *cap);

/* void nresize(SCRN *t,int w,int h);
 *
 * Change size of screen.  For example, call this when you find out that
 * the Xterm changed size.
 */
void nresize(SCRN *t, int w, int h);

/* void nredraw(SCRN *t);
 *
 * Invalidate all state variables for the terminal.  This way, everything gets
 * redrawn.
 */
void nredraw(SCRN *t);

void npartial(SCRN *t);
void nescape(SCRN *t);
void nreturn(SCRN *t);

/* void nclose(SCRN *t);
 *
 * Close the screen and restore TTY to initial state.
 *
 * if 'flg' is set, tclose doesn't mess with the signals.
 */
void nclose(SCRN *t);

/* int cpos(SCRN *t,int x,int y);
 *
 * Set cursor position
 */
int cpos(register SCRN *t, register int x, register int y);

/* int attr(SCRN *t,int a);
 *
 * Set attributes
 */
int set_attr(SCRN *t, int c);

/* Encode character as utf8 */
void utf8_putc(int c);

/* void outatr(SCRN *t,int *scrn,int *attr,int x,int y,int c,int a);
 *
 * Output a character at the given screen cooridinate.  The cursor position
 * after this function is executed is indeterminate.
 */

/* Character attribute bits */

#define INVERSE		 256
#define UNDERLINE	 512
#define BOLD		1024
#define BLINK		2048
#define DIM		4096
#define AT_MASK		(INVERSE+UNDERLINE+BOLD+BLINK+DIM)

#define BG_SHIFT 13
#define BG_VALUE (7<<BG_SHIFT)
#define BG_NOT_DEFAULT (8<<BG_SHIFT)
#define BG_MASK (15<<BG_SHIFT)

#define BG_DEFAULT (0<<BG_SHIFT) /* default */
#define BG_BLACK (8<<BG_SHIFT)
#define BG_RED (9<<BG_SHIFT)
#define BG_GREEN (10<<BG_SHIFT)
#define BG_YELLOW (11<<BG_SHIFT)
#define BG_BLUE (12<<BG_SHIFT)
#define BG_MAGENTA (13<<BG_SHIFT)
#define BG_CYAN (14<<BG_SHIFT)
#define BG_WHITE (15<<BG_SHIFT)

#define FG_SHIFT 17
#define FG_VALUE (7<<FG_SHIFT)
#define FG_NOT_DEFAULT (8<<FG_SHIFT)
#define FG_MASK (15<<FG_SHIFT)

#define FG_DEFAULT (0<<FG_SHIFT)
#define FG_WHITE (8<<FG_SHIFT) /* default */
#define FG_CYAN (9<<FG_SHIFT)
#define FG_MAGENTA (10<<FG_SHIFT)
#define FG_BLUE (11<<FG_SHIFT)
#define FG_YELLOW (12<<FG_SHIFT)
#define FG_GREEN (13<<FG_SHIFT)
#define FG_RED (14<<FG_SHIFT)
#define FG_BLACK (15<<FG_SHIFT)

#define HAS_COMBINING 0x200000

void outatr(struct charmap *map,SCRN *t,int *scrn,int *attrf,int xx,int yy,int c,int a);
void outatr_help(SCRN *, int *, int *, int, int, int, int);

/* int eraeol(SCRN *t,int x,int y);
 *
 * Erase from screen coordinate to end of line.
 */
int eraeol(SCRN *t, int x, int y);

/* void nscrlup(SCRN *t,int top,int bot,int amnt);
 *
 * Buffered scroll request.  Request that some lines up.  'top' and 'bot'
 * indicate which lines to scroll.  'bot' is the last line to scroll + 1.
 * 'amnt' is distance in lines to scroll.
 */
void nscrlup(SCRN *t, int top, int bot, int amnt);

/* void nscrldn(SCRN *t,int top,int bot,int amnt);
 *
 * Buffered scroll request.  Scroll some lines down.  'top' and 'bot'
 * indicate which lines to scroll.  'bot' is the last line to scroll + 1.
 * 'amnt' is distance in lines to scroll.
 */
void nscrldn(SCRN *t, int top, int bot, int amnt);

/* void nscroll(SCRN *t);
 *
 * Execute buffered scroll requests
 */
void nscroll(SCRN *t);

/* void magic(SCRN *t,int y,int *cur,int *new);
 *
 * Figure out and execute line shifting
 */
void magic(SCRN *t, int y, int *cs, int *ca, int *s, int *a,int placex);

int clrins(SCRN *t);

int meta_color(unsigned char *s);

/* Generate a field */
void genfield(SCRN *t,int *scrn,int *attr,int x,int y,int ofst,unsigned char *s,int len,int atr,int width,int flg,int *fmt);

/* Column width of a string takes into account utf-8) */
int txtwidth(unsigned char *s,int len);

/* Generate a field: formatted */
void genfmt(SCRN *t, int x, int y, int ofst, const unsigned char *s, int flg);

/* Column width of formatted string */
int fmtlen(const unsigned char *s);

/* Offset within formatted string of particular column */
int fmtpos(unsigned char *s, int goal);

#endif
