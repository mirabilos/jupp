/*
 *	Window management
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#ifndef _JOE_W_H
#define _JOE_W_H 1

#ifdef EXTERN
__IDSTRING(rcsid_w_h, "$MirOS: contrib/code/jupp/w.h,v 1.9 2017/12/07 02:10:20 tg Exp $");
#endif

/***************/
/* Subroutines */
/***************/

/* int getgrouph(W *);
 * Get height of a family of windows
 */
int getgrouph(W *w);

/* W *findtopw(W *);
 * Find first (top-most) window of a family
 */
W *findtopw(W *w);

/* W *findbotw(W *);
 * Find last (bottom-most) window a family
 */
W *findbotw(W *w);

int demotegroup(W *w);

/* W *lastw(SCREEN *t);
 * Find last window on screen
 */
W *lastw(SCREEN *t);

/* Determine number of main windows
 */
int countmain(SCREEN *t);

/* void wfit(SCREEN *t);
 *
 * Fit all of the windows onto the screen
 */
void wfit(SCREEN *t);

/*****************/
/* Main routines */
/*****************/

/* SCREEN *screate(SCRN *);
 *
 * Create a screen
 */
SCREEN *screate(SCRN *scrn);

/* void sresize(SCREEN *t);
 * Screen size changed
 */
void sresize(SCREEN *t);

/* W *wcreate(SCREEN *t,WATOM *watom,W *where,W *target,W *original,int height);
 *
 * Try to create a window
 *
 * 't'		Is the screen the window is placed on
 * 'watom'	Type of new window
 * 'where'	The window is placed after this window, or if 'where'==0, the
 *		window is placed on the end of the screen
 * 'target'	The window operates on this window.  The window becomes a
 *		member of 'target's family or starts a new family if
 *		'target'==0.
 * 'original'	Attempt to get 'height' from this window.  When the window is
 *              aborted, the space gets returned to 'original' if it still
 *		exists.  If 'original'==0, the window will force other
 *		windows to go off of the screen.
 * 'height'	The height of the window
 *
 * Returns the new window or returns 0 if there was not enough space to
 * create the window and maintain family integrity.
 */
W *wcreate(SCREEN *t, WATOM *watom, W *where, W *target, W *original, int height, const unsigned char *huh, int *notify);

/* int wabort(W *w);
 *
 * Kill a window and it's children
 */
int wabort(W *w);

/* int wnext(SCREEN *);
 *
 * Switch to next window
 */
int wnext(SCREEN *t);

/* int wprev(SCREEN *);
 *
 * Switch to previous window
 */
int wprev(SCREEN *t);

/* int wgrow(W *);
 *
 * increase size of window.  Return 0 for success, -1 for fail.
 */
int wgrow(W *w);

/* int wshrink(W *);
 *
 * Decrease size of window.  Returns 0 for success, -1 for fail.
 */
int wshrink(W *w);

/* void wshowone(W *);
 *
 * Show only one window on the screen
 */
void wshowone(W *w);

/* void wshowall(SCREEN *);
 *
 * Show all windows on the screen, including the given one
 */
void wshowall(SCREEN *t);

/* void wredraw(W *);
 *
 * Force complete redraw of window
 */
void wredraw(W *w);

/* void updall()
 *
 * Redraw all windows
 */
void updall(void);

/* void msgnw[t](W *w, char *s);
 * Display a message which will be eliminated on the next keypress.
 * msgnw displays message on bottom line of window
 * msgnwt displays message on top line of window
 */
void msgnw(W *w, const unsigned char *s);
void msgnwt(W *w, const unsigned char *s);

/* Message composition buffer for msgnw/msgnwt */
#define JOE_MSGBUFSIZE 300
extern unsigned char msgbuf[JOE_MSGBUFSIZE];

void msgout(W *w);			/* Output msgnw/msgnwt messages */

/* Common user functions */

int urtn(jobject jO, int k);		/* User hit return */
int utype(jobject jO, int k);		/* User types a character */
int uretyp(BASE *bw);			/* Refresh the screen */
int ugroww(BASE *bw);			/* Grow current window */
int uexpld(BASE *bw);			/* Explode current window or show all windows */
int ushrnk(BASE *bw);			/* Shrink current window */
int unextw(BASE *bw);			/* Goto next window */
int uprevw(BASE *bw);			/* Goto previous window */

void scrdel(B *b, long int l, long int n, int flg);
void scrins(B *b, long int l, long int n, int flg);

#endif
