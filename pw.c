/*
 *	Prompt windows
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#include "config.h"
#include "types.h"

#include <string.h>

#include "b.h"
#include "bw.h"
#include "help.h"
#include "kbd.h"
#include "pw.h"
#include "scrn.h"
#include "tab.h"
#include "termcap.h"
#include "tw.h"
#include "uedit.h"
#include "undo.h"
#include "utils.h"
#include "vfile.h"
#include "menu.h"
#include "va.h"
#include "w.h"

extern int smode;
extern int beep;

static void disppw(BW *bw, int flg)
{
	W *w = bw->parent;
	PW *pw = (PW *) bw->object;

	if (!flg) {
		return;
	}

	/* Scroll buffer and position prompt */
	if (pw->promptlen > w->w / 2 + w->w / 4) {
		pw->promptofst = pw->promptlen - w->w / 2;
		if (piscol(bw->cursor) < w->w - (pw->promptlen - pw->promptofst)) {
			bw->offset = 0;
		} else {
			bw->offset = piscol(bw->cursor) - (w->w - (pw->promptlen - pw->promptofst) - 1);
		}
	} else {
		if (piscol(bw->cursor) < w->w - pw->promptlen) {
			pw->promptofst = 0;
			bw->offset = 0;
		} else if (piscol(bw->cursor) >= w->w) {
			pw->promptofst = pw->promptlen;
			bw->offset = piscol(bw->cursor) - (w->w - 1);
		} else {
			pw->promptofst = pw->promptlen - (w->w - piscol(bw->cursor) - 1);
			bw->offset = piscol(bw->cursor) - (w->w - (pw->promptlen - pw->promptofst) - 1);
		}
	}

	/* Set cursor position */
	w->curx = piscol(bw->cursor) - bw->offset + pw->promptlen - pw->promptofst;
	w->cury = 0;

	/* Generate prompt */
	w->t->t->updtab[w->y] = 1;
	genfmt(w->t->t, w->x, w->y, pw->promptofst, pw->prompt, 0);

	/* Position and size buffer */
	bwmove(bw, w->x + pw->promptlen - pw->promptofst, w->y);
	bwresz(bw, w->w - (pw->promptlen - pw->promptofst), 1);

	/* Generate buffer */
	bwgen(bw, 0);
}

/* When user hits return in a prompt window */

extern volatile int dostaupd;

static int rtnpw(BW *bw)
{
	W *w = bw->parent;
	PW *pw = (PW *) bw->object;
	unsigned char *s;
	W *win;
	int *notify;
	int (*pfunc) ();
	void *object;
	long byte;

	p_goto_eol(bw->cursor);
	byte = bw->cursor->byte;
	p_goto_bol(bw->cursor);
	s = brvs(bw->cursor, (int) (byte - bw->cursor->byte));
	if (pw->hist) {
		if (bw->b->changed) {
			P *q = pdup(pw->hist->eof);

			binsm(q, s, (int) (byte - bw->cursor->byte));
			p_goto_eof(q);
			binsc(q, '\n');
			prm(q);
		} else {
			P *q = pdup(pw->hist->bof);
			P *r;
			P *t;

			pline(q, bw->cursor->line);
			r = pdup(q);
			pnextl(r);
			t = pdup(pw->hist->eof);
			binsb(t, bcpy(q, r));
			bdel(q, r);
			prm(q);
			prm(r);
			prm(t);
		}
	}
	win = w->win;
	pfunc = pw->pfunc;
	object = pw->object;
	bwrm(bw);
	joe_free(pw->prompt);
	joe_free(pw);
	w->object = NULL;
	notify = w->notify;
	w->notify = 0;
	wabort(w);
	dostaupd = 1;
	if (pfunc) {
		return pfunc(win->object, s, object, notify);
	} else {
		return -1;
	}
}

int ucmplt(BW *bw, int k)
{
	PW *pw = (PW *) bw->object;

	if (pw->tab) {
		return pw->tab(bw, k);
	} else {
		return -1;
	}
}

static void inspw(BW *bw, B *b, long l, long n, int flg)
{
	if (b == bw->b) {
		bwins(bw, l, n, flg);
	}
}

static void delpw(BW *bw, B *b, long l, long n, int flg)
{
	if (b == bw->b) {
		bwdel(bw, l, n, flg);
	}
}

static int abortpw(BW *b)
{
	PW *pw = b->object;
	void *object = pw->object;
	int (*abrt) () = pw->abrt;

	W *win = b->parent->win;

	bwrm(b);
	joe_free(pw->prompt);
	joe_free(pw);
	if (abrt) {
		return abrt(win->object, object);
	} else {
		return -1;
	}
}

static WATOM watompw = {
	US "prompt",
	disppw,
	bwfllw,
	abortpw,
	rtnpw,
	utypebw,
	NULL,
	NULL,
	inspw,
	delpw,
	TYPEPW
};

/* Create a prompt window */

BW *wmkpw(W *w, unsigned char *prompt, B **history, int (*func) (), unsigned char *huh, int (*abrt) (), int (*tab) (), void *object, int *notify,struct charmap *map)
{
	W *new;
	PW *pw;
	BW *bw;

	new = wcreate(w->t, &watompw, w, w, w->main, 1, huh, notify);
	if (!new) {
		if (notify) {
			*notify = 1;
		}
		return NULL;
	}
	wfit(new->t);
	new->object = (void *) (bw = bwmk(new, bmk(NULL), 1));
	bw->b->o.charmap = map;
	bw->object = (void *) (pw = (PW *) joe_malloc(sizeof(PW)));
	pw->abrt = abrt;
	pw->tab = tab;
	pw->object = object;
	pw->prompt = (unsigned char *)strdup((char *)prompt);
	pw->promptlen = fmtlen(prompt);
	pw->promptofst = 0;
	pw->pfunc = func;
	if (history) {
		if (!*history) {
			*history = bmk(NULL);
		}
		pw->hist = *history;
		binsb(bw->cursor, bcpy(pw->hist->bof, pw->hist->eof));
		bw->b->changed = 0;
		p_goto_eof(bw->cursor);
		p_goto_eof(bw->top);
		p_goto_bol(bw->top);
	} else {
		pw->hist = NULL;
	}
	w->t->curwin = new;
	return bw;
}

/* Tab completion functions */

unsigned char **regsub(unsigned char **z, int len, unsigned char *s)
{
	unsigned char **lst = NULL;
	int x;

	for (x = 0; x != len; ++x)
		if (rmatch(s, z[x]))
			lst = vaadd(lst, vsncpy(NULL, 0, sz(z[x])));
	return lst;
}

void cmplt_ins(BW *bw, unsigned char *line)
{
	P *p = pdup(bw->cursor);

	p_goto_bol(p);
	p_goto_eol(bw->cursor);
	bdel(p, bw->cursor);
	binsm(bw->cursor, sv(line));
	p_goto_eol(bw->cursor);
	prm(p);
	bw->cursor->xcol = piscol(bw->cursor);
}

int cmplt_abrt(BW *bw, int x, unsigned char *line)
{
	if (line) {
		cmplt_ins(bw, line);
		vsrm(line);
	}
	return -1;
}

int cmplt_rtn(MENU *m, int x, unsigned char *line)
{
	cmplt_ins(m->parent->win->object, m->list[x]);
	vsrm(line);
	m->object = NULL;
	wabort(m->parent);
	return 0;
}

int simple_cmplt(BW *bw,unsigned char **list)
{
	MENU *m;
	P *p, *q;
	unsigned char *line;
	unsigned char *line1;
	unsigned char **lst;

	p = pdup(bw->cursor);
	p_goto_bol(p);
	q = pdup(bw->cursor);
	p_goto_eol(q);
	line = brvs(p, (int) (q->byte - p->byte));	/* Assumes short lines :-) */
	prm(p);
	prm(q);

	line1 = vsncpy(NULL, 0, sv(line));
	line1 = vsadd(line1, '*');
	lst = regsub(list, aLEN(list), line1);
	vsrm(line1);

	if (!lst) {
		ttputc(7);
		vsrm(line);
		return -1;
	}

	m = mkmenu(bw->parent, lst, cmplt_rtn, cmplt_abrt, NULL, 0, line, NULL);
	if (!m) {
		varm(lst);
		vsrm(line);
		return -1;
	}
	if (aLEN(lst) == 1)
		return cmplt_rtn(m, 0, line);
	else if (smode || isreg(line))
		return 0;
	else {
		unsigned char *com = mcomplete(m);

		vsrm(m->object);
		m->object = com;
		wabort(m->parent);
		smode = 2;
		ttputc(7);
		return 0;
	}
}
