/*
 *	Shell-window functions
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#include "config.h"
#include "types.h"

__RCSID("$MirOS: contrib/code/jupp/ushell.c,v 1.15 2017/12/08 03:24:16 tg Exp $");

#include <sys/stat.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

#include "b.h"
#include "main.h"
#include "pw.h"
#include "qw.h"
#include "tty.h"
#include "uedit.h"
#include "uerror.h"
#include "ufile.h"
#include "va.h"
#include "vs.h"
#include "ushell.h"
#include "utf8.h"
#include "w.h"

extern int orphan;

#if WANT_FORK
/* Executed when shell process terminates */

static void cdone(B *b)
{
	b->pid = 0;
	close(b->out);
	b->out = -1;
}

static void cdone_parse(B *b)
{
	b->pid = 0;
	close(b->out);
	b->out = -1;
	parserrb(b);
}

/* Executed for each chunk of data we get from the shell */

static void cfollow(B *b,long byte)
{
	W *w;
	if ((w = maint->topwin) != NULL) {
		do {
			if ((w->watom->what & TYPETW) &&
			    w->object.bw->b == b &&
			    w->object.bw->cursor->byte == byte) {
				BW *bw = w->object.bw;
				p_goto_eof(bw->cursor);
				bw->cursor->xcol = piscol(bw->cursor);
			}
			w = w->link.next;
		} while (w != maint->topwin);
	}
}

static void cdata(B *b, unsigned char *dat, int siz)
{
	P *q = pdup(b->eof);
	P *r = pdup(b->eof);
	long byte = q->byte;
	unsigned char bf[1024];
	int x, y;

	for (x = y = 0; x != siz; ++x) {
		if (dat[x] == 13 || dat[x] == 0) {
			;
		} else if (dat[x] == 8 || dat[x] == 127) {
			if (y) {
				--y;
			} else {
				pset(q, r);
				prgetc(q);
				bdel(q, r);
				--byte;
			}
		} else if (dat[x] == 7) {
			ttputc(7);
		} else {
			bf[y++] = dat[x];
		}
	}
	if (y) {
		binsm(r, bf, y);
	}
	prm(r);
	prm(q);

	cfollow(b,byte);
}

static int doushell(BW *bw, unsigned char *cmd, int *notify, int build)
{
	MPX *m;
	unsigned char **s;
	unsigned char *u;
	const unsigned char *name;

	name = getushell();
	s = vamk(10);
	u = vsncpy(NULL, 0, sz(name));
	s = vaadd(s, u);
	if (cmd) {
		u = vsncpy(NULL, 0, sc("-c"));
		s = vaadd(s, u);
		s = vaadd(s, cmd);
	} else {
		u = vsncpy(NULL, 0, sc("-i"));
		s = vaadd(s, u);
	}

	if (notify) {
		*notify = 1;
	}
	if (bw->b->pid) {
		msgnw(bw->parent, UC "Program already running in this window");
		varm(s);
		vsrm(cmd);
		return -1;
	}
	p_goto_eof(bw->cursor);

	if (!(m = mpxmk(&bw->b->out, name, s, cdata, bw->b, build ? cdone_parse : cdone, bw->b))) {
		varm(s);
		vsrm(cmd);
		msgnw(bw->parent, UC "No ptys available");
		return -1;
	} else {
		bw->b->pid = m->pid;
	}
	varm(s);
	vsrm(cmd);
	return 0;
}

int ubknd(BW *bw)
{
	if (!getenv("SHELL")) {
		msgnw(bw->parent, UC "\"SHELL\" environment variable not defined or exported");
	}
	return doushell(bw, NULL, NULL, 0);
}

/* Run a program in a window */

static int dorun(BW *bw, unsigned char *s, void *object, int *notify)
{
	return doushell(bw, s, notify, 0);
}

B *runhist = NULL;

int urun(BW *bw)
{
	if (wmkpw(bw->parent, UC "Program to run: ", &runhist, dorun, UC "Run", NULL, NULL, NULL, NULL, locale_map)) {
		return 0;
	} else {
		return -1;
	}
}

static int dobuild(BW *bw, unsigned char *s, void *object, int *notify)
{
	return doushell(bw, s, notify, 1);
}

B *buildhist = NULL;

int ubuild(BW *bw)
{
	if (buildhist) {
		if ((bw=wmkpw(bw->parent, UC "Build command: ", &buildhist, dobuild, UC "Run", NULL, NULL, NULL, NULL, locale_map))) {
			uuparw(bw);
			u_goto_eol(bw);
			bw->cursor->xcol = piscol(bw->cursor);
			return 0;
		}
	} else if (wmkpw(bw->parent, UC "Enter build command (for example, 'make'): ", &buildhist, dobuild, UC "Run", NULL, NULL, NULL, NULL, locale_map))
			return 0;
		return -1;
}
#endif

/* Kill program */

static int pidabort(BW *bw, int c, void *object, int *notify)
{
	if (notify) {
		*notify = 1;
	}
	if ((c | 0x20) != 'y') {
		return -1;
	}
	if (bw->b->pid) {
		kill(bw->b->pid, 1);
		return -1;
	} else {
		return -1;
	}
}

int ukillpid(BW *bw)
{
	if (bw->b->pid) {
		if (mkqw(bw->parent, sc("Kill program (y,n,^C)?"), pidabort, NULL, NULL, NULL)) {
			return 0;
		} else {
			return -1;
		}
	} else {
		return 0;
	}
}

static const char * const getushell_envs[] = {
	"SHELL",
	"EXECSHELL",
};
const void *getushell(void)
{
	static char *rshell;

	if (!rshell) {
		char *eshell;
		struct stat sbuf;
		int i = 0;

		while (i < 2) {
			eshell = getenv(getushell_envs[i++]);
			if (eshell && *eshell &&
			    !stat(eshell, &sbuf) &&
			    S_ISREG(sbuf.st_mode) &&
			    (sbuf.st_mode & 0111) &&
			    /* LINTED use of access */
			    !access(eshell, X_OK)) {
				rshell = eshell;
				break;
			}
		}
	}
	return (rshell ? rshell : "/bin/sh");
}
