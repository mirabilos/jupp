/*
 *	Incremental search
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#include "config.h"
#include "types.h"

__RCSID("$MirOS: contrib/code/jupp/uisrch.c,v 1.16 2020/03/27 06:08:17 tg Exp $");

#include <stdlib.h>

#include "b.h"
#include "bw.h"
#include "main.h"
#include "queue.h"
#include "qw.h"
#include "tty.h"
#include "uisrch.h"
#include "usearch.h"
#include "utils.h"
#include "charmap.h"
#include "vs.h"

extern int smode;
extern int dobeep;
extern int icase;
struct isrch *lastisrch = NULL;	/* Previous search */

unsigned char *lastpat = NULL;	/* Previous pattern */

extern SRCH *globalsrch;	/* Existing SRCH structure */

IREC fri = { {&fri, &fri}, 0, 0, 0, 0 };	/* Free-list of irecs */

static IREC *alirec(void)
{				/* Allocate an IREC */
	return alitem(&fri, sizeof(IREC));
}

static void frirec(IREC *i)
{				/* Free an IREC */
	enquef(IREC, link, &fri, i);
}

static void rmisrch(struct isrch *isrch)
{				/* Eliminate a struct isrch */
	if (isrch) {
		vsrm(isrch->pattern);
		vsrm(isrch->prompt);
		frchn(&fri, &isrch->irecs);
		free(isrch);
	}
}

static int iabrt(BW *bw, struct isrch *isrch)
{				/* User hit ^C */
	rmisrch(isrch);
	return -1;
}

static void iappend(BW *bw, struct isrch *isrch, unsigned char *s, int len)
{				/* Append text and search */
	/* Append char and search */
	IREC *i = alirec();
	SRCH *srch;

	i->what = len;
	i->disp = bw->cursor->byte;
	isrch->pattern = vsncpy(sv(isrch->pattern), s, len);
	if (!qempty(IREC, link, &isrch->irecs)) {
		pgoto(bw->cursor, isrch->irecs.link.prev->start);
	}
	i->start = bw->cursor->byte;

	if (!globalsrch)
		srch = mksrch(NULL,NULL,icase,isrch->dir,-1,0,0);
	else {
		srch = globalsrch;
		globalsrch = 0;
	}

	srch->addr = bw->cursor->byte;

	if (!srch->wrap_p || srch->wrap_p->b!=bw->b) {
		prm(srch->wrap_p);
		srch->wrap_p = pdup(bw->cursor);
		srch->wrap_p->owner = &srch->wrap_p;
		srch->wrap_flag = 0;
	}

	i->wrap_flag = srch->wrap_flag;

	vsrm(srch->pattern);
	srch->pattern = vsncpy(NULL, 0, isrch->pattern, sLen(isrch->pattern));
	srch->backwards = isrch->dir;

	if (dopfnext(bw, srch, NULL)) {
		if(dobeep)
			ttputc(7);
	}
	enqueb(IREC, link, &isrch->irecs, i);
}

/* Main user interface */
/* When called with c==-1, it just creates the prompt */
static int itype(BW *bw, int c, struct isrch *isrch, int *notify)
{
	IREC *i;
	int omid;
	int tc;

	if (isrch->quote) {
		goto in;
	}
	if (c == 8 || c == 127) {	/* Backup */
		if ((i = isrch->irecs.link.prev) != &isrch->irecs) {
			pgoto(bw->cursor, i->disp);
			if (globalsrch)
				globalsrch->wrap_flag = i->wrap_flag;
			omid = mid;
			mid = 1;
			dofollows();
			mid = omid;
			isrch->pattern = vstrunc(isrch->pattern, sLEN(isrch->pattern) - i->what);
			frirec(deque_f(IREC, link, i));
		} else {
			if(dobeep)
				ttputc(7);
		}
	} else if (c == 'Q' - '@' || c == '`') {
		isrch->quote = 1;
	} else if (c == 'S' - '@' || c == '\\' - '@' || c == 'L' - '@' || c == 'R' - '@') {
		/* Repeat */
		if (c == 'R' - '@') {
			isrch->dir = 1;
		} else {
			isrch->dir = 0;
		}
		if (qempty(IREC, link, &isrch->irecs)) {
			if (lastpat && lastpat[0]) {
				iappend(bw, isrch, sv(lastpat));
			}
		} else {
			SRCH *srch;
			i = alirec();
			i->disp = i->start = bw->cursor->byte;
			i->what = 0;

			if (!globalsrch)
				srch = mksrch(NULL,NULL,icase,isrch->dir,-1,0,0);
			else {
				srch = globalsrch;
				globalsrch = 0;
			}

			srch->addr = bw->cursor->byte;

			if (!srch->wrap_p || srch->wrap_p->b!=bw->b) {
				prm(srch->wrap_p);
				srch->wrap_p = pdup(bw->cursor);
				srch->wrap_p->owner = &srch->wrap_p;
				srch->wrap_flag = 0;
			}

			i->wrap_flag = srch->wrap_flag;

			vsrm(srch->pattern);
			srch->pattern = vsncpy(NULL, 0, isrch->pattern, sLen(isrch->pattern));
			srch->backwards = isrch->dir;

			if (dopfnext(bw, srch, NULL)) {
				if(dobeep)
					ttputc(7);
				frirec(i);
			} else {
				enqueb(IREC, link, &isrch->irecs, i);
			}
		}
	} else if (c >= 0 && c < 32) {
		/* Done when a control character is received */
		nungetc(c);
		if (notify) {
			*notify = 1;
		}
		smode = 2;
		if (lastisrch) {
			lastpat = vstrunc(lastpat, 0);
			lastpat = vsncpy(lastpat, 0, lastisrch->pattern, sLen(lastisrch->pattern));
			rmisrch(lastisrch);
		}
		lastisrch = isrch;
		return 0;
	} else if (c != -1) {
		unsigned char buf[16];
		int buf_len;
		/* Search */
 in:
		/* Convert to/from utf-8 */
		if (joe_maputf(locale_map) && !joe_maputf(bw->b->o.charmap)) {
			utf8_encode(buf,c);
			c = from_utf8(bw->b->o.charmap,buf);
		} else if (!joe_maputf(locale_map) && joe_maputf(bw->b->o.charmap)) {
			to_utf8(locale_map,buf,c);
			c = utf8_decode_string(buf);
		}

		if (joe_maputf(bw->b->o.charmap)) {
			buf_len = utf8_encode(buf,c);
		} else {
			buf[0] = c;
			buf_len = 1;
		}

		isrch->quote = 0;
		iappend(bw, isrch, buf, buf_len);
	}
	omid = mid;
	mid = 1;
	bw->cursor->xcol = piscol(bw->cursor);
	dofollows();
	mid = omid;

	isrch->prompt = vstrunc(isrch->prompt, isrch->ofst);

	if (joe_maputf(locale_map) && !joe_maputf(bw->b->o.charmap)) {
		/* Translate bytes to utf-8 */
		unsigned char buf[16];
		int x;

		for (x = 0; x != sLEN(isrch->pattern); ++x) {
			tc = joe_to_uni(bw->b->o.charmap, isrch->pattern[x]);
			utf8_encode(buf, tc);
			isrch->prompt = vsncpy(sv(isrch->prompt), sz(buf));
		}
	} else if (!joe_maputf(locale_map) && joe_maputf(bw->b->o.charmap)) {
		/* Translate utf-8 to bytes */
		unsigned char *p = isrch->pattern;
		int len = sLEN(isrch->pattern);

		while (len) {
			if ((tc = utf8_decode_fwrd(&p, &len)) >= 0) {
				tc = joe_from_uni(locale_map, tc);
				isrch->prompt = vsadd(isrch->prompt, tc);
			}
		}
	} else {
		/* FIXME: translate when charmaps do not match */
		isrch->prompt = vsncpy(sv(isrch->prompt),sv(isrch->pattern));
	}

	if (mkqwnsr(bw->parent, sv(isrch->prompt), itype, iabrt, isrch, notify)) {
		return 0;
	} else {
		rmisrch(isrch);
		return -1;
	}
}

/* Create a struct isrch */
static int
doisrch(BW *bw, int dir)
{
	struct isrch *isrch = malloc(sizeof(struct isrch));

	izque(IREC, link, &isrch->irecs);
	isrch->pattern = vsncpy(NULL, 0, NULL, 0);
	isrch->dir = dir;
	isrch->quote = 0;
	isrch->prompt = vsncpy(NULL, 0, sc("I-find: "));
	isrch->ofst = sLen(isrch->prompt);
	return itype(bw, -1, isrch, NULL);
}

int uisrch(BW *bw)
{
	if (smode && lastisrch) {
		struct isrch *isrch = lastisrch;

		lastisrch = 0;
		return itype(bw, 'S' - '@', isrch, NULL);
	} else {
		if (globalsrch) {
			rmsrch(globalsrch);
			globalsrch = 0;
		}
		if (lastisrch) {
			lastpat = vstrunc(lastpat, 0);
			lastpat = vsncpy(lastpat, 0, lastisrch->pattern, sLen(lastisrch->pattern));
			rmisrch(lastisrch);
			lastisrch = 0;
		}
		return doisrch(bw, 0);
	}
}

int ursrch(BW *bw)
{
	if (smode && lastisrch) {
		struct isrch *isrch = lastisrch;

		lastisrch = 0;
		return itype(bw, 'R' - '@', isrch, NULL);
	} else {
		if (globalsrch) {
			rmsrch(globalsrch);
			globalsrch = 0;
		}
		if (lastisrch) {
			lastpat = vstrunc(lastpat, 0);
			lastpat = vsncpy(lastpat, 0, lastisrch->pattern, sLen(lastisrch->pattern));
			rmisrch(lastisrch);
			lastisrch = 0;
		}
		return doisrch(bw, 1);
	}
}
