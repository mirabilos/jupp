/*
 *	Editor startup and main edit loop
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 * 	This file is part of JOE (Joe's Own Editor)
 */
#include "config.h"
#include "types.h"

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#include "b.h"
#include "help.h"
#include "kbd.h"
#include "macro.h"
#include "path.h"
#include "rc.h"
#include "scrn.h"
#include "termcap.h"
#include "tw.h"
#include "vfile.h"
#include "vs.h"
#include "w.h"
#include "utf8.h"
#include "charmap.h"
#include "syntax.h"

extern int mid, dspasis, force, help, pgamnt, nobackups, lightoff, exask, skiptop, noxon, lines, staen, columns, Baud, dopadding, marking, beep;

extern int idleout;		/* Clear to use /dev/tty for screen */
extern unsigned char *joeterm;
int help = 0;			/* Set to have help on when starting */
int nonotice = 0;		/* Set to prevent copyright notice */
int orphan = 0;
unsigned char *exmsg = NULL;		/* Message to display when exiting the editor */

SCREEN *maint;			/* Main edit screen */

/* Make windows follow cursor */

void dofollows(void)
{
	W *w = maint->curwin;

	do {
		if (w->y != -1 && w->watom->follow && w->object)
			w->watom->follow(w->object);
		w = (W *) (w->link.next);
	} while (w != maint->curwin);
}

/* Update screen */

int dostaupd = 1;
extern int staupd;

void edupd(int flg)
{
	W *w;
	int wid, hei;

	if (dostaupd) {
		staupd = 1;
		dostaupd = 0;
	}
	ttgtsz(&wid, &hei);
	if ((wid >= 2 && wid != maint->w) || (hei >= 1 && hei != maint->h)) {
		nresize(maint->t, wid, hei);
		sresize(maint);
	}
	dofollows();
	ttflsh();
	nscroll(maint->t);
	help_display(maint);
	w = maint->curwin;
	do {
		if (w->y != -1) {
			if (w->object && w->watom->disp)
				w->watom->disp(w->object, flg);
			msgout(w);
		}
		w = (W *) (w->link.next);
	} while (w != maint->curwin);
	cpos(maint->t, maint->curwin->x + maint->curwin->curx, maint->curwin->y + maint->curwin->cury);
	staupd = 0;
}

static int ahead = 0;
static int ungot = 0;
static int ungotc = 0;

void nungetc(int c)
{
	if (c != 'C' - '@' && c != 'M' - '@') {
		chmac();
		ungot = 1;
		ungotc = c;
	}
}

int edloop(int flg)
{
	int term = 0;
	int ret = 0;

	if (flg) {
		if (maint->curwin->watom->what == TYPETW)
			return 0;
		else
			maint->curwin->notify = &term;
	}
	while (!leave && (!flg || !term)) {
		MACRO *m;
		int c;

		if (exmsg && !flg) {
			vsrm(exmsg);
			exmsg = NULL;
		}
		edupd(1);
		if (!ahead && !have)
			ahead = 1;
		if (ungot) {
			c = ungotc;
			ungot = 0;
		} else
			c = ttgetc();

		if (!ahead && c == 10)
			c = 13;
		m = dokey(maint->curwin->kbd, c);
		if (maint->curwin->main && maint->curwin->main != maint->curwin) {
			int x = maint->curwin->kbd->x;

			maint->curwin->main->kbd->x = x;
			if (x)
				maint->curwin->main->kbd->seq[x - 1] = maint->curwin->kbd->seq[x - 1];
		}
		if (m)
			ret = exemac(m);
	}

	if (term == -1)
		return -1;
	else
		return ret;
}

#ifdef __MSDOS__
extern void setbreak();
extern int breakflg;
#endif

unsigned char **mainenv;

int main(int argc, unsigned char **argv, unsigned char **envv)
{
	CAP *cap;
	unsigned char *s;
	unsigned char *run;
#ifdef __MSDOS__
	unsigned char *rundir;
#endif
	SCRN *n;
	int opened = 0;
	int omid;
	int backopt;
	int c;

	joe_locale();

	mainenv = envv;

#ifdef __MSDOS__
	_fmode = O_BINARY;
	strcpy(stdbuf, argv[0]);
	joesep(stdbuf);
	run = namprt(stdbuf);
	rundir = dirprt(stdbuf);
	for (c = 0; run[c]; ++c)
		if (run[c] == '.') {
			run = vstrunc(run, c);
			break;
		}
#else
	run = namprt(argv[0]);
#endif

	if ((s = (unsigned char *)getenv("LINES")) != NULL)
		sscanf((char *)s, "%d", &lines);
	if ((s = (unsigned char *)getenv("COLUMNS")) != NULL)
		sscanf((char *)s, "%d", &columns);
	if ((s = (unsigned char *)getenv("BAUD")) != NULL)
		sscanf((char *)s, "%u", &Baud);
	if (getenv("DOPADDING"))
		dopadding = 1;
	if (getenv("NOXON"))
		noxon = 1;
	if ((s = (unsigned char *)getenv("JOETERM")) != NULL)
		joeterm = s;

#ifndef __MSDOS__
	if (!(cap = getcap(NULL, 9600, NULL, NULL))) {
		fprintf(stderr, "Couldn't load termcap/terminfo entry\n");
		return 1;
	}
#endif

#ifdef __MSDOS__

	s = vsncpy(NULL, 0, sv(run));
	s = vsncpy(sv(s), sc("rc"));
	c = procrc(cap, s);
	if (c == 0)
		goto donerc;
	if (c == 1) {
		unsigned char buf[8];

		fprintf(stderr, "There were errors in '%s'.  Use it anyway?", s);
		fflush(stderr);
		fgets(buf, 8, stdin);
		if (buf[0] == 'y' || buf[0] == 'Y')
			goto donerc;
	}

	vsrm(s);
	s = vsncpy(NULL, 0, sv(rundir));
	s = vsncpy(sv(s), sv(run));
	s = vsncpy(sv(s), sc("rc"));
	c = procrc(cap, s);
	if (c == 0)
		goto donerc;
	if (c == 1) {
		unsigned char buf[8];

		fprintf(stderr, "There were errors in '%s'.  Use it anyway?", s);
		fflush(stderr);
		fgets(buf, 8, stdin);
		if (buf[0] == 'y' || buf[0] == 'Y')
			goto donerc;
	}
#else

	s = (unsigned char *)getenv("HOME");
	if (s) {
		s = vsncpy(NULL, 0, sz(s));
		s = vsncpy(sv(s), sc("/."));
		s = vsncpy(sv(s), sv(run));
		s = vsncpy(sv(s), sc("rc"));
		c = procrc(cap, s);
		if (c == 0)
			goto donerc;
		if (c == 1) {
			unsigned char buf[8];

			fprintf(stderr, "There were errors in '%s'.  Use it anyway?", s);
			fflush(stderr);
			fgets((char *)buf, 8, stdin);
			if (buf[0] == 'y' || buf[0] == 'Y')
				goto donerc;
		}
	}

	vsrm(s);
	s = vsncpy(NULL, 0, sc(JOERC));
	s = vsncpy(sv(s), sv(run));
	s = vsncpy(sv(s), sc("rc"));
	c = procrc(cap, s);
	if (c == 0)
		goto donerc;
	if (c == 1) {
		unsigned char buf[8];

		fprintf(stderr, "There were errors in '%s'.  Use it anyway?", s);
		fflush(stderr);
		fgets((char *)buf, 8, stdin);
		if (buf[0] == 'y' || buf[0] == 'Y')
			goto donerc;
	}
#endif

	fprintf(stderr, "Couldn't open '%s'\n", s);
	return 1;

      donerc:
	help_init(s);
	for (c = 1; argv[c]; ++c) {
		if (argv[c][0] == '-') {
			if (argv[c][1])
				switch (glopt(argv[c] + 1, argv[c + 1], NULL, 1)) {
				case 0:
					fprintf(stderr, "Unknown option '%s'\n", argv[c]);
					break;
				case 1:
					break;
				case 2:
					++c;
					break;
			} else
				idleout = 0;
		}
	}

	if (!(n = nopen(cap)))
		return 1;
	maint = screate(n);
	vmem = vtmp();

	for (c = 1, backopt = 0; argv[c]; ++c)
		if (argv[c][0] == '+' && argv[c][1]) {
			if (!backopt)
				backopt = c;
		} else if (argv[c][0] == '-' && argv[c][1]) {
			if (!backopt)
				backopt = c;
			if (glopt(argv[c] + 1, argv[c + 1], NULL, 0) == 2)
				++c;
		} else {
			B *b = bfind(argv[c]);
			BW *bw = NULL;
			int er = error;

			if (!orphan || !opened) {
				bw = wmktw(maint, b);
				if (er)
					msgnwt(bw->parent, msgs[-er]);
			} else
				b->orphan = 1;
			if (bw) {
				long lnum = 0;

				bw->o.readonly = bw->b->rdonly;
				if (backopt) {
					while (backopt != c) {
						if (argv[backopt][0] == '+') {
							sscanf((char *)(argv[backopt] + 1), "%ld", &lnum);
							++backopt;
						} else {
							if (glopt(argv[backopt] + 1, argv[backopt + 1], &bw->o, 0) == 2)
								backopt += 2;
							else
								backopt += 1;
							lazy_opts(&bw->o);
						}
					}
				}
				bw->b->o = bw->o;
				bw->b->rdonly = bw->o.readonly;
				if (!opened)
					maint->curwin = bw->parent;
				if (er == -1 && bw->o.mnew)
					exemac(bw->o.mnew);
				if (er == 0 && bw->o.mold)
					exemac(bw->o.mold);
				if (lnum > 0)
					pline(bw->cursor, lnum - 1);
			}
			opened = 1;
			backopt = 0;
		}

	if (opened) {
		wshowall(maint);
		omid = mid;
		mid = 1;
		dofollows();
		mid = omid;
	} else {
		BW *bw = wmktw(maint, bfind(US ""));

		if (bw->o.mnew)
			exemac(bw->o.mnew);
	}
	maint->curwin = maint->topwin;

	if (help) {
		help_on(maint);
	}
	if (!nonotice) {
		if (locale_map->type)
			joe_snprintf_1((char *)msgbuf,JOE_MSGBUFSIZE,"\\i** Joe's Own Editor v" VERSION " ** (%s) ** Copyright © 2004 **\\i",locale_map->name);
		else
			joe_snprintf_1((char *)msgbuf,JOE_MSGBUFSIZE,"\\i** Joe's Own Editor v" VERSION " ** (%s) ** Copyright (C) 2004 **\\i",locale_map->name);

		msgnw(((BASE *)lastw(maint)->object)->parent, msgbuf);
	}

	edloop(0);
	vclose(vmem);
	nclose(n);
	if (exmsg)
		fprintf(stderr, "\n%s\n", exmsg);
	return 0;
}
