/*
 * 	Highlighted block functions
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#include "config.h"
#include "types.h"

__RCSID("$MirOS: contrib/code/jupp/ublock.c,v 1.32 2018/11/11 18:15:39 tg Exp $");

#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>

#include "b.h"
#include "pw.h"
#include "queue.h"
#include "scrn.h"
#include "tty.h"
#include "ublock.h"
#include "uedit.h"
#include "utils.h"
#include "vs.h"
#include "path.h"
#include "poshist.h"
#include "ushell.h"
#include "charmap.h"
#include "w.h"

/* Global options */

int square = 0;			/* Set for rectangle mode */
int lightoff = 0;		/* Set if highlighting should turn off

				   after block operations */
extern int marking;

/* Global variables */

P *markb = NULL;		/* Beginning and end of block */
P *markk = NULL;

/* Push markb & markk */

typedef struct marksav MARKSAV;
struct marksav {
	LINK(MARKSAV) link;
	P *markb, *markk;
} markstack = { { &markstack, &markstack}, NULL, NULL };
MARKSAV markfree = { {&markfree, &markfree}, NULL, NULL };
int nstack = 0;

int upsh(BW *bw)
{
	MARKSAV *m = alitem(&markfree, sizeof(MARKSAV));

	m->markb = 0;
	m->markk = 0;
	if (markk)
		pdupown(markk, &m->markk);
	if (markb)
		pdupown(markb, &m->markb);
	enqueb(MARKSAV, link, &markstack, m);
	++nstack;
	return 0;
}

int upop(BW *bw)
{
	MARKSAV *m = markstack.link.prev;

	if (m != &markstack) {
		--nstack;
		prm(markk);
		prm(markb);
		markk = m->markk;
		if (markk)
			markk->owner = &markk;
		markb = m->markb;
		if (markb)
			markb->owner = &markb;
		demote(MARKSAV, link, &markfree, m);
		if (lightoff)
			unmark(bw);
		updall();
		return 0;
	} else
		return -1;
}

/* Return true if markb/markk are valid */
/* If r is set, swap markb with markk if necessary */

int autoswap;

int markv(int r)
{
	if (markb && markk && markb->b == markk->b && markk->byte > markb->byte && (!square || markk->xcol > markb->xcol)) {
		return 1;
	} else if(autoswap && r && markb && markk && markb->b == markk->b && markb->byte > markk->byte && (!square || markk->xcol < markb->xcol)) {
		P *p = pdup(markb);
		prm(markb); markb=0; pdupown(markk, &markb);
		prm(markk); markk=0; pdupown(p, &markk);
		prm(p);
		return 1;
	} else
		return 0;
}

/* Rectangle-mode subroutines */

/* B *pextrect(P *org,long height,long left,long right);
 * Copy a rectangle into a new buffer
 *
 * org points to top-left corner of rectangle.
 * height is number of lines in rectangle.
 * right is rightmost column of rectangle + 1
 */

B *pextrect(P *org, long int height, long int right)
{
	P *p = pdup(org);	/* Left part of text to extract */
	P *q = pdup(p);		/* After right part of text to extract */
	B *tmp = bmk(NULL);	/* Buffer to extract to */
	P *z = pdup(tmp->eof);	/* Buffer pointer */

	while (height--) {
		pcol(p, org->xcol);
		pset(q, p);
		pcolwse(q, right);
		p_goto_eof(z);
		binsb(z, bcpy(p, q));
		p_goto_eof(z);
		binsc(z, '\n');
		pnextl(p);
	}
	prm(p);
	prm(q);
	prm(z);
	return tmp;
}

/* void pdelrect(P *org,long height,long right);
 * Delete a rectangle.
 */

void pdelrect(P *org, long int height, long int right)
{
	P *p = pdup(org);
	P *q = pdup(p);

	while (height--) {
		pcol(p, org->xcol);
		pset(q, p);
		pcol(q, right);
		bdel(p, q);
		pnextl(p);
	}
	prm(p);
	prm(q);
}

/* void pclrrect(P *org,long height,long right,int usetabs);
 * Blank-out a rectangle.
 */

void pclrrect(P *org, long int height, long int right, int usetabs)
{
	P *p = pdup(org);
	P *q = pdup(p);

	while (height--) {
		long pos;

		pcol(p, org->xcol);
		pset(q, p);
		pcoli(q, right);
		pos = q->col;
		bdel(p, q);
		pfill(p, pos, usetabs);
		pnextl(p);
	}
	prm(p);
	prm(q);
}

/* int ptabrect(P *org,long height,long right)
 * Check if there are any TABs in a rectangle
 */

int ptabrect(P *org, long int height, long int right)
{
	P *p = pdup(org);

	while (height--) {
		int c;

		pcol(p, org->xcol);
		while ((c = pgetc(p)) != NO_MORE_DATA && c != '\n') {
			if (c == '\t') {
				prm(p);
				return '\t';
			} else if (piscol(p) > right)
				break;
		}
		if (c != '\n')
			pnextl(p);
	}
	prm(p);
	return ' ';
}

/* Insert rectangle */

void pinsrect(P *cur, B *tmp, long int width, int usetabs)
{
	P *p = pdup(cur);	/* We insert at & move this pointer */
	P *q = pdup(tmp->bof);	/* These are for scanning through 'tmp' */
	P *r = pdup(q);

	if (width)
		while (pset(r, q), p_goto_eol(q), (q->line != tmp->eof->line || piscol(q))) {
			pcol(p, cur->xcol);
			if (piscol(p) < cur->xcol)
				pfill(p, cur->xcol, usetabs);
			binsb(p, bcpy(r, q));
			pfwrd(p, q->byte - r->byte);
			if (piscol(p) < cur->xcol + width)
				pfill(p, cur->xcol + width, usetabs);
			if (piseol(p))
				pbackws(p);
			if (!pnextl(p)) {
				binsc(p, '\n');
				pgetc(p);
			}
			if (pgetc(q) == NO_MORE_DATA)
				break;
		}
	prm(p);
	prm(q);
	prm(r);
}

/* Block functions */

/* Set beginning */

int umarkb(BW *bw)
{
	pdupown(bw->cursor, &markb);
	markb->xcol = bw->cursor->xcol;
	updall();
	return 0;
}

int udrop(BW *bw)
{
	prm(markk);
	if (marking && markb)
		prm(markb);
	else
		umarkb(bw);
	return 0;
}

int ubegin_marking(BW *bw)
{
	if (marking)
		/* We're marking now... don't stop */
		return 0;
	else if (markv(0) && bw->cursor->b==markb->b) {
		/* Try to extend current block */
		if (bw->cursor->byte==markb->byte) {
			pset(markb,markk);
			prm(markk); markk=0;
			marking = 1;
			return 0;
		} else if(bw->cursor->byte==markk->byte) {
			prm(markk); markk=0;
			marking = 1;
			return 0;
		}
	}
	/* Start marking - no message */
	prm(markb); markb=0;
	prm(markk); markk=0;
	updall();
	marking = 1;
	return umarkb(bw);
}

int utoggle_marking(BW *bw)
{
	if (markv(0) && bw->cursor->b==markb->b && bw->cursor->byte>=markb->byte && bw->cursor->byte<=markk->byte) {
		/* Just clear selection */
		prm(markb); markb=0;
		prm(markk); markk=0;
		updall();
		marking = 0;
		msgnw(bw->parent, UC "Selection cleared.");
		return 0;
	} else if (markk) {
		/* Clear selection and start new one */
		prm(markb); markb=0;
		prm(markk); markk=0;
		updall();
		marking = 1;
		msgnw(bw->parent, UC "Selection started.");
		return umarkb(bw);
	} else if (markb && markb->b==bw->cursor->b) {
		marking = 0;
		if (bw->cursor->byte<markb->byte) {
			pdupown(markb, &markk);
			prm(markb); markb=0;
			pdupown(bw->cursor, &markb);
			markb->xcol = bw->cursor->xcol;
		} else {
			pdupown(bw->cursor, &markk);
			markk->xcol = bw->cursor->xcol;
		}
		updall(); /* Because other windows could be changed */
		return 0;
	} else {
		marking = 1;
		msgnw(bw->parent, UC "Selection started.");
		return umarkb(bw);
	}
}

int uselect(BW *bw)
{
	if (!markb)
		umarkb(bw);
	return 0;
}

/* Set end */

int umarkk(BW *bw)
{
	pdupown(bw->cursor, &markk);
	markk->xcol = bw->cursor->xcol;
	updall();
	return 0;
}

/* Unset marks */

int unmark(BW *bw)
{
	prm(markb);
	prm(markk);
	updall();
	return 0;
}

/* Mark line */

int umarkl(BW *bw)
{
	p_goto_bol(bw->cursor);
	umarkb(bw);
	pnextl(bw->cursor);
	umarkk(bw);
	utomarkb(bw);
	pcol(bw->cursor, bw->cursor->xcol);
	return 0;
}

int utomarkb(BW *bw)
{
	if (markb && markb->b == bw->b) {
		pset(bw->cursor, markb);
		return 0;
	} else
		return -1;
}

int utomarkk(BW *bw)
{
	if (markk && markk->b == bw->b) {
		pset(bw->cursor, markk);
		return 0;
	} else
		return -1;
}

int uswap(BW *bw)
{
	if (markb && markb->b == bw->b) {
		P *q = pdup(markb);

		umarkb(bw);
		pset(bw->cursor, q);
		prm(q);
		return 0;
	} else
		return -1;
}

int utomarkbk(BW *bw)
{
	if (markb && markb->b == bw->b && bw->cursor->byte != markb->byte) {
		pset(bw->cursor, markb);
		return 0;
	} else if (markk && markk->b == bw->b && bw->cursor->byte != markk->byte) {
		pset(bw->cursor, markk);
		return 0;
	} else
		return -1;
}

/* Delete block */

int ublkdel(BW *bw)
{
	if (markv(1)) {
		if (square)
			if (bw->o.overtype) {
				long ocol = markk->xcol;

				pclrrect(markb, markk->line - markb->line + 1, markk->xcol, ptabrect(markb, markk->line - markb->line + 1, markk->xcol));
				pcol(markk, ocol);
				markk->xcol = ocol;
			} else
				pdelrect(markb, markk->line - markb->line + 1, markk->xcol);
		else
			bdel(markb, markk);
		if (lightoff)
			unmark(bw);
	} else {
		msgnw(bw->parent, UC "No block");
		return -1;
	}
	return 0;
}

/* Special delete block function for PICO */

int upicokill(BW *bw)
{
	upsh(bw);
	umarkk(bw);
	if (markv(1)) {
		if (square)
			if (bw->o.overtype) {
				long ocol = markk->xcol;

				pclrrect(markb, markk->line - markb->line + 1, markk->xcol, ptabrect(markb, markk->line - markb->line + 1, markk->xcol));
				pcol(markk, ocol);
				markk->xcol = ocol;
			} else
				pdelrect(markb, markk->line - markb->line + 1, markk->xcol);
		else
			bdel(markb, markk);
		if (lightoff)
			unmark(bw);
	} else
		udelln(bw);
	return 0;
}

/* Move highlighted block */

int ublkmove(BW *bw)
{
	if (markv(1)) {
		if (markb->b->rdonly) {
			msgnw(bw->parent, UC "Read only");
			return -1;
		}
		if (square) {
			long height = markk->line - markb->line + 1;
			long width = markk->xcol - markb->xcol;
			int usetabs = ptabrect(markb, height, markk->xcol);
			long ocol = piscol(bw->cursor);
			B *tmp = pextrect(markb, height, markk->xcol);
			int update_xcol = (bw->cursor->xcol >= markk->xcol && bw->cursor->line >= markb->line && bw->cursor->line <= markk->line);

			ublkdel(bw);
			/* now we can't use markb and markk until we set them again */
			/* ublkdel() frees them */
			if (bw->o.overtype) {
				/* If cursor was in block, blkdel moves it to left edge of block, so fix it
				 * back to its original place here */
				pcol(bw->cursor, ocol);
				pfill(bw->cursor, ocol, ' ');
				pdelrect(bw->cursor, height, piscol(bw->cursor) + width);
			} else if (update_xcol)
				/* If cursor was to right of block, xcol was not properly updated */
				bw->cursor->xcol -= width;
			pinsrect(bw->cursor, tmp, width, usetabs);
			brm(tmp);
			if (lightoff)
				unmark(bw);
			else {
				umarkb(bw);
				umarkk(bw);
				pline(markk, markk->line + height - 1);
				pcol(markk, markb->xcol + width);
				markk->xcol = markb->xcol + width;
			}
			return 0;
		} else if (bw->cursor->b != markk->b || bw->cursor->byte > markk->byte || bw->cursor->byte < markb->byte) {
			long size = markk->byte - markb->byte;

			binsb(bw->cursor, bcpy(markb, markk));
			bdel(markb, markk);
			if (lightoff)
				unmark(bw);
			else {
				umarkb(bw);
				umarkk(bw);
				pfwrd(markk, size);
			}
			updall();
			return 0;
		}
	}
	msgnw(bw->parent, UC "No block");
	return -1;
}

/* Duplicate highlighted block */

int ublkcpy(BW *bw)
{
	if (markv(1)) {
		if (square) {
			long height = markk->line - markb->line + 1;
			long width = markk->xcol - markb->xcol;
			int usetabs = ptabrect(markb, height, markk->xcol);
			B *tmp = pextrect(markb, height, markk->xcol);

			if (bw->o.overtype)
				pdelrect(bw->cursor, height, piscol(bw->cursor) + width);
			pinsrect(bw->cursor, tmp, width, usetabs);
			brm(tmp);
			if (lightoff)
				unmark(bw);
			else {
				umarkb(bw);
				umarkk(bw);
				pline(markk, markk->line + height - 1);
				pcol(markk, markb->xcol + width);
				markk->xcol = markb->xcol + width;
			}
			return 0;
		} else {
			long size = markk->byte - markb->byte;
			B *tmp = bcpy(markb, markk);

			/* Simple overtype for hex mode */
			if (bw->o.hex && bw->o.overtype) {
				P *q = pdup(bw->cursor);
				if (q->byte + size >= q->b->eof->byte)
					pset(q, q->b->eof);
				else
					pfwrd(q, size);
				bdel(bw->cursor, q);
				prm(q);
			}

			binsb(bw->cursor, tmp);
			if (lightoff)
				unmark(bw);
			else {
				umarkb(bw);
				umarkk(bw);
				pfwrd(markk, size);
			}
			updall();
			return 0;
		}
	} else {
		msgnw(bw->parent, UC "No block");
		return -1;
	}
}

/* Write highlighted block to a file */
/* This is called by ublksave in ufile.c */

int dowrite(BW *bw, unsigned char *s, void *object, int *notify)
{
	int fl;
	int ret = 0;

	if (notify)
		*notify = 1;
	if (!markv(1)) {
		vsrm(s);
		msgnw(bw->parent, UC "No block");
		return (-1);
	}
	if (square) {
		B *tmp = pextrect(markb,
				  markk->line - markb->line + 1,
				  markk->xcol);

		fl = bsave(tmp->bof, s, tmp->eof->byte, 0);
		brm(tmp);
	} else {
		fl = bsave(markb, s, markk->byte - markb->byte, 0);
	}
	if (fl != 0) {
		msgnw(bw->parent, msgs[-fl]);
		ret = -1;
	}
	if (lightoff)
		unmark(bw);
	vsrm(s);
	return (ret);
}

/* Set highlighted block on a program block */

void setindent(BW *bw)
{
	P *p, *q;
	long indent;

	if (pisblank(bw->cursor))
		return;

	p = pdup(bw->cursor);
	q = pdup(p);
	indent = pisindent(p);

	do {
		if (!pprevl(p))
			goto done;
		else
			p_goto_bol(p);
	} while (pisindent(p) >= indent || pisblank(p));
	pnextl(p);
	/* Maybe skip blank lines at beginning */
 done:
	p_goto_bol(p);
	p->xcol = piscol(p);
	if (markb)
		prm(markb);
	markb = p;
	p->owner = &markb;

	do {
		if (!pnextl(q))
			break;
	} while (pisindent(q) >= indent || pisblank(q));
	/* Maybe skip blank lines at end */
	if (markk)
		prm(markk);
	q->xcol = piscol(q);
	markk = q;
	q->owner = &markk;

	updall();
}

/* Purity check */
/* Verifies that at least n indentation characters (for non-blank lines) match c */
/* If n is 0 (for urindent), this fails if c is space but indentation begins with tab */

static int
purity_check(int c, int n)
{
	P *p = pdup(markb);
	while (p->byte < markk->byte) {
		int x;
		p_goto_bol(p);
		if (!n && c==' ' && brc(p)=='\t') {
			prm(p);
			return 0;
		} else if (!piseol(p))
			for (x=0; x!=n; ++x)
				if (pgetc(p)!=c) {
					prm(p);
					return 0;
				}
		pnextl(p);
	}
	prm(p);
	return 1;
}

/* Left indent check */
/* Verify that there is enough whitespace to do the left indent */

static int
lindent_check(int c, int n)
{
	P *p = pdup(markb);
	int indwid;
	if (c=='\t')
		indwid = n * p->b->o.tab;
	else
		indwid = n;
	while (p->byte < markk->byte) {
		p_goto_bol(p);
		if (!piseol(p) && pisindent(p)<indwid) {
			prm(p);
			return 0;
		}
		pnextl(p);
	}
	prm(p);
	return 1;
}

/* Indent more */

int urindent(BW *bw)
{
	if (square) {
		if (markb && markk && markb->b == markk->b && markb->byte <= markk->byte && markb->xcol <= markk->xcol) {
			P *p = pdup(markb);

			do {
				pcol(p, markb->xcol);
				pfill(p, markb->xcol + bw->o.istep, bw->o.indentc);
			} while (pnextl(p) && p->line <= markk->line);
			prm(p);
		}
	} else {
		if (!markb || !markk || markb->b != markk->b || bw->cursor->byte < markb->byte || bw->cursor->byte > markk->byte || markb->byte == markk->byte) {
			setindent(bw);
		} else if ( 1 /* bw->o.purify */) {
			P *p = pdup(markb);
			P *q = pdup(markb);
			int indwid;

			if (bw->o.indentc=='\t')
				indwid = bw->o.tab * bw->o.istep;
			else
				indwid = bw->o.istep;

			while (p->byte < markk->byte) {
				p_goto_bol(p);
				if (!piseol(p)) {
					int col;
					pset(q, p);
					p_goto_indent(q, bw->o.indentc);
					col = piscol(q);
					bdel(p,q);
					pfill(p,col+indwid,bw->o.indentc);
				}
				pnextl(p);
			}
			prm(p);
			prm(q);
		} else if (purity_check(bw->o.indentc,0)) {
			P *p = pdup(markb);

			while (p->byte < markk->byte) {
				p_goto_bol(p);
				if (!piseol(p))
					while (piscol(p) < bw->o.istep) {
						binsc(p, bw->o.indentc);
						pgetc(p);
					}
				pnextl(p);
			}
			prm(p);
		} else {
			/* Purity failure */
			msgnw(bw->parent,UC "Selected lines not properly indented");
			return 1;
		}
	}
	return 0;
}

/* Indent less */

int ulindent(BW *bw)
{
	if (square) {
		if (markb && markk && markb->b == markk->b && markb->byte <= markk->byte && markb->xcol <= markk->xcol) {
			P *p = pdup(markb);
			P *q = pdup(p);

			do {
				pcol(p, markb->xcol);
				while (piscol(p) < markb->xcol + bw->o.istep) {
					int c = pgetc(p);

					if (c != ' ' && c != '\t' && c != bw->o.indentc) {
						prm(p);
						prm(q);
						return -1;
					}
				}
			} while (pnextl(p) && p->line <= markk->line);
			pset(p, markb);
			do {
				pcol(p, markb->xcol);
				pset(q, p);
				pcol(q, markb->xcol + bw->o.istep);
				bdel(p, q);
			} while (pnextl(p) && p->line <= markk->line);
			prm(p);
			prm(q);
		}
	} else {
		if (!markb || !markk || markb->b != markk->b || bw->cursor->byte < markb->byte || bw->cursor->byte > markk->byte || markb->byte == markk->byte) {
			setindent(bw);
		} else if (1 /* bw->o.purify */ && lindent_check(bw->o.indentc,bw->o.istep)) {
			P *p = pdup(markb);
			P *q = pdup(markb);
			int indwid;

			if (bw->o.indentc=='\t')
				indwid = bw->o.tab * bw->o.istep;
			else
				indwid = bw->o.istep;

			while (p->byte < markk->byte) {
				p_goto_bol(p);
				if (!piseol(p)) {
					int col;
					pset(q, p);
					p_goto_indent(q, bw->o.indentc);
					col = piscol(q);
					bdel(p,q);
					pfill(p,col-indwid,bw->o.indentc);
				}
				pnextl(p);
			}
			prm(p);
			prm(q);
		} else if (purity_check(bw->o.indentc,bw->o.istep)) {
			P *p = pdup(markb);
			P *q = pdup(p);

			p_goto_bol(p);
			while (p->byte < markk->byte) {
				if (!piseol(p)) {
					pset(q, p);
					while (piscol(q) < bw->o.istep)
						pgetc(q);
					bdel(p, q);
				}
				pnextl(p);
			}
			prm(p);
			prm(q);
		} else {
			/* Purity failure */
			msgnw(bw->parent, UC "Selected lines not properly indented");
			return 1;
		}
	}
	return 0;
}

/* Insert a file */

int doinsf(BW *bw, unsigned char *s, void *object, int *notify)
{
	if (notify)
		*notify = 1;
	if (square) {
		if (markv(1)) {
			B *tmp;
			long width = markk->xcol - markb->xcol;
			long height;
			int usetabs = ptabrect(markb,
					       markk->line - markb->line + 1,
					       markk->xcol);

			tmp = bload(s);
			if (error) {
				msgnw(bw->parent, msgs[-error]);
				brm(tmp);
				vsrm(s);
				return -1;
			}
			if (piscol(tmp->eof))
				height = tmp->eof->line + 1;
			else
				height = tmp->eof->line;
			if (bw->o.overtype) {
				pclrrect(markb, long_max(markk->line - markb->line + 1, height), markk->xcol, usetabs);
				pdelrect(markb, height, width + markb->xcol);
			}
			pinsrect(markb, tmp, width, usetabs);
			pdupown(markb, &markk);
			markk->xcol = markb->xcol;
			if (height) {
				pline(markk, markk->line + height - 1);
				pcol(markk, markb->xcol + width);
				markk->xcol = markb->xcol + width;
			}
			brm(tmp);
			updall();
			vsrm(s);
			return 0;
		} else {
			vsrm(s);
			msgnw(bw->parent, UC "No block");
			return -1;
		}
	} else {
		int ret = 0;
		B *tmp = bload(s);

		if (error) {
			msgnw(bw->parent, msgs[-error]), brm(tmp);
			ret = -1;
		} else {
			P *pafter;

			pafter = pdup(bw->cursor);
			pgetc(pafter);
			binsb(bw->cursor, tmp);
			prgetc(pafter);
			aftermove(bw->parent, pafter);
		}
		vsrm(s);
		bw->cursor->xcol = piscol(bw->cursor);
		return ret;
	}
}


/* Filter highlighted block through a UNIX command */

static int filtflg = 0;

#if WANT_FORK
#define v_or_fork() fork()
#else
#define v_or_fork() vfork()
#endif

/*
 * This isn't optimal, but until the home-brewn VM system is removed
 * it is the best we can do: we cannot use bsavefd() in a concurrent
 * child because it uses JOE's VM subsystem which then copies around
 * content in file-backed memory that's not unshared, leading to da-
 * ta corruption if the content is big enough.
 *
 * TBH, I'd rather love to see that VM system gone and revert to the
 * JOE original code for dofilt... --mirabilos
 */
static int dofilt(BW *bw, unsigned char *s, void *object, int *notify)
{
	int fr[2];
	int fw;
	volatile int flg = 0;
	unsigned char *tf;
	const char *sh;
#if defined(HAVE_PUTENV) && (WANT_FORK || defined(HAVE_UNSETENV))
	unsigned char *fname;
#endif

	if (notify)
		*notify = 1;
	if (markb && markk && !square && markb->b == bw->b && markk->b == bw->b && markb->byte == markk->byte) {
		flg = 1;
		goto ok;
	} if (!markv(1)) {
		vsrm(s);
		msgnw(bw->parent, UC "No block");
		return -1;
	}
 ok:
	if (pipe(fr)) {
		vsrm(s);
		msgnw(bw->parent, UC "Pipe error");
		return (-1);
	}
	if ((tf = mktmp(NULL, &fw)) == NULL) {
		msgnw(bw->parent, UC "Cannot create temporary file");
 lseekoops:
		close(fr[0]);
		close(fr[1]);
		vsrm(s);
		return (-1);
	}
	unlink((char *)tf);
	vsrm(tf);
	npartial(bw->parent->t->t);
	ttclsn();
	if (square) {
		B *tmp = pextrect(markb,
				  markk->line - markb->line + 1,
				  markk->xcol);

		bsavefd(tmp->bof, fw, tmp->eof->byte);
	} else
		bsavefd(markb, fw, markk->byte - markb->byte);
	if (lseek(fw, (off_t)0, SEEK_SET) < 0) {
		msgnw(bw->parent, UC "lseek failed");
		close(fw);
		goto lseekoops;
	}
#if defined(HAVE_PUTENV) && (WANT_FORK || defined(HAVE_UNSETENV))
	fname = vsncpy(NULL, 0, sc("JOE_FILENAME="));
	tf = bw->b->name ? bw->b->name : (unsigned char *)"Unnamed";
	fname = vsncpy(sv(fname), sz(tf));
#if !WANT_FORK
	putenv((char *)fname);
#endif
#endif
	sh = getushell();
	if (!v_or_fork()) {
#if defined(HAVE_PUTENV) && WANT_FORK
		putenv((char *)fname);
#endif
		signrm(1);
		close(0);
		close(1);
		close(2);
		/* these dups will not fail */
		if (dup(fw)) {}
		if (dup(fr[1])) {}
		if (dup(fr[1])) {}
		close(fw);
		close(fr[1]);
		close(fr[0]);
		execl(sh, sh, "-c", s, NULL);
		_exit(0);
	}
	close(fr[1]);
	close(fw);
#if defined(HAVE_PUTENV) && (WANT_FORK || defined(HAVE_UNSETENV))
#if !WANT_FORK
	unsetenv("JOE_FILENAME");
#endif
	vsrm(fname);
#endif
	if (square) {
		B *tmp;
		long width = markk->xcol - markb->xcol;
		long height;
		int usetabs = ptabrect(markb,
				       markk->line - markb->line + 1,
				       markk->xcol);

		tmp = bread(fr[0], LONG_MAX);
		if (piscol(tmp->eof))
			height = tmp->eof->line + 1;
		else
			height = tmp->eof->line;
		if (bw->o.overtype) {
			pclrrect(markb, markk->line - markb->line + 1, markk->xcol, usetabs);
			pdelrect(markb, long_max(height, markk->line - markb->line + 1), width + markb->xcol);
		} else
			pdelrect(markb, markk->line - markb->line + 1, markk->xcol);
		pinsrect(markb, tmp, width, usetabs);
		pdupown(markb, &markk);
		markk->xcol = markb->xcol;
		if (height) {
			pline(markk, markk->line + height - 1);
			pcol(markk, markb->xcol + width);
			markk->xcol = markb->xcol + width;
		}
		if (lightoff)
			unmark(bw);
		brm(tmp);
		updall();
	} else {
		P *p = pdup(markk);
		if (!flg)
			prgetc(p);
		bdel(markb, p);
		binsb(p, bread(fr[0], LONG_MAX));
		if (!flg) {
			pset(p,markk);
			prgetc(p);
			bdel(p,markk);
		}
		prm(p);
		if (lightoff)
			unmark(bw);
	}
	close(fr[0]);
	wait(NULL);
	vsrm(s);
	ttopnn();
	if (filtflg)
		unmark(bw);
	bw->cursor->xcol = piscol(bw->cursor);
	return 0;
}

static B *filthist = NULL;

static void markall(BW *bw)
{
	pdupown(bw->cursor->b->bof, &markb);
	markb->xcol = 0;
	pdupown(bw->cursor->b->eof, &markk);
	markk->xcol = piscol(markk);
	updall();
}

static int checkmark(BW *bw)
{
	if (!markv(1))
		if (square)
			return 2;
		else {
			markall(bw);
			filtflg = 1;
			return 1;
	} else {
		filtflg = 0;
		return 0;
	}
}

int ufilt(BW *bw)
{
	switch (checkmark(bw)) {
	case 0:
		if (wmkpw(bw->parent, UC "Command to filter block through (^C to abort): ", &filthist, dofilt, NULL, NULL, utypebw, NULL, NULL, locale_map))
			return 0;
		else
			return -1;
	case 1:
		if (wmkpw(bw->parent, UC "Command to filter file through (^C to abort): ", &filthist, dofilt, NULL, NULL, utypebw, NULL, NULL, locale_map))
			return 0;
		else
			return -1;
	case 2:
	default:
		msgnw(bw->parent, UC "No block");
		return -1;
	}
}

/* Force region to lower case */

int ulower(BW *bw)
{
	if (markv(1)) {
		P *q;
		P *p;
		int c;
		B *b = bcpy(markb,markk);
		/* Leave one character in buffer to keep pointers set properly... */
		q = pdup(markk);
		prgetc(q);
		bdel(markb,q);
		b->o.charmap = markb->b->o.charmap;
		p=pdup(b->bof);
		while ((c=pgetc(p))!=NO_MORE_DATA) {
			c = joe_tolower(b->o.charmap,c);
			binsc(q,c);
			pgetc(q);
		}
		prm(p);
		bdel(q,markk);
		prm(q);
		brm(b);
		bw->cursor->xcol = piscol(bw->cursor);
		return 0;
	} else
		return -1;
}

/* Force region to upper case */

int uupper(BW *bw)
{
	if (markv(1)) {
		P *q;
		P *p;
		int c;
		B *b = bcpy(markb,markk);
		q = pdup(markk);
		prgetc(q);
		bdel(markb,q);
		b->o.charmap = markb->b->o.charmap;
		p=pdup(b->bof);
		while ((c=pgetc(p))!=NO_MORE_DATA) {
			c = joe_toupper(b->o.charmap,c);
			binsc(q,c);
			pgetc(q);
		}
		prm(p);
		bdel(q,markk);
		prm(q);
		brm(b);
		bw->cursor->xcol = piscol(bw->cursor);
		return 0;
	} else
		return -1;
}
