/*
 *	User text formatting functions
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#include "config.h"
#include "types.h"

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include "b.h"
#include "ublock.h"
#include "charmap.h"
#include "utils.h"

/* Center line cursor is on and move cursor to beginning of next line */

int ucenter(BW *bw)
{
	P *p = bw->cursor, *q;
	long endcol, begcol, x;
	int c;

	p_goto_eol(p);
	while (joe_isblank(bw->b->o.charmap, (c = prgetc(p))))
		/* do nothing */;
	if (c == '\n') {
		pgetc(p);
		goto done;
	}
	if (c == NO_MORE_DATA)
		goto done;
	pgetc(p);
	endcol = piscol(p);

	p_goto_bol(p);
	while (joe_isblank(bw->b->o.charmap, (c = pgetc(p))))
		/* do nothing */;
	if (c == '\n') {
		prgetc(p);
		goto done;
	}
	if (c == NO_MORE_DATA)
		goto done;
	prgetc(p);
	begcol = piscol(p);

	if (endcol - begcol > bw->o.rmargin + bw->o.lmargin)
		goto done;

	q = pdup(p);
	p_goto_bol(q);
	bdel(q, p);
	prm(q);

	for (x = 0; x != (bw->o.lmargin + bw->o.rmargin) / 2 - (endcol - begcol) / 2; ++x)
		binsc(p, ' ');

      done:
	if (!pnextl(p)) {
		binsc(p, '\n');
		pgetc(p);
		return -1;
	} else
		return 0;
}

/* Return true if c is a character which can indent a paragraph */

static int cpara(int c)
{
	if (c == ' ' || c == '\t' || c == '\\' ||
	    c == '>' || c == '|' || c == ':' || c == '*' || c == '/' ||
	    c == ',' || c == '.' || c == '?' || c == ';' || c == ']' ||
	    c == '}' || c == '=' || c == '+' || c == '-' || c == '_' ||
	    c == ')' || c == '&' || c == '^' || c == '%' || c == '$' ||
	    c == '#' || c == '@' || c == '!' || c == '~')
		return 1;
	else
		return 0;
}

/* Return true if line is definitly not a paragraph line.
 * Lines which arn't paragraph lines:
 *  1) Blank lines
 *  2) Lines which begin with '.'
 */

static int pisnpara(P *p)
{
	P *q;
	int c;

	q = pdup(p);
	p_goto_bol(q);
	while (cpara(c = pgetc(q)))
		/* do nothing */;
	prm(q);
	if (c == '.' || c == '\r' || c == '\n')
		return 1;
	else
		return 0;
}

/* Determine amount of indentation on current line */

static long nindent(P *p)
{
	P *q = pdup(p);
	long col;

	p_goto_bol(q);
	do {
		col = q->col;
	} while (cpara(pgetc(q)));
	prm(q);
	return col;
}

/* Get indentation prefix column */

static long prefix(P *p)
{
	long len;
	P *q = pdup(p);

	p_goto_bol(q);
	while (cpara(brch(q)))
		pgetc(q);
	while (!pisbol(q))
		if (!joe_isblank(p->b->o.charmap, prgetc(q))) {
			pgetc(q);
			break;
		}
	len = q->col;
	prm(q);
	return len;
}

/* Move pointer to beginning of paragraph
 *
 * This function simply moves backwards until it sees:
 *  0) The beginning of the file
 *  1) A blank line
 *  2) A line with a different indentation prefix
 *  3) A line with indentation greater than that of the line we started with
 *  4) A line with indentation less than that of the starting line, but with
 *     a blank line (or beginning of file) preceeding it.
 */

int within = 0;

P *pbop(P *p)
{
	long indent;
	long prelen;

	p_goto_bol(p);
	indent = nindent(p);
	prelen = prefix(p);
	while (!pisbof(p) && (!within || !markb || p->byte > markb->byte)) {
		long ind;
		long len;

		pprevl(p);
		p_goto_bol(p);
		ind = nindent(p);
		len = prefix(p);
		if (pisnpara(p) || len != prelen) {
			pnextl(p);
			break;
		}
		if (ind > indent)
			break;
		if (ind < indent) {
			if (pisbof(p))
				break;
			pprevl(p);
			p_goto_bol(p);
			if (pisnpara(p)) {
				pnextl(p);
				break;
			} else {
				pnextl(p);
				pnextl(p);
				break;
			}
		}
	}
	return p;
}

/* Move pointer to end of paragraph.  Pointer must already be on first
 * line of paragraph for this to work correctly.
 *
 * This function moves forwards until it sees:
 *  0) The end of the file.
 *  1) A blank line
 *  2) A line with indentation different from the second line of the paragraph
 *  3) A line with prefix column different from first line
 */

P *peop(P *p)
{
	long indent;
	long prelen;

	if (!pnextl(p) || pisnpara(p) || (within && markk && p->byte >= markk->byte))
		return p;
	indent = nindent(p);
	prelen = prefix(p);
	while (pnextl(p) && (!within || !markk || p->byte < markk->byte)) {
		long ind = nindent(p);
		long len = prefix(p);

		if (ind != indent || len != prelen || pisnpara(p))
			break;
	}
	return p;
}

/* Motion commands */

int ubop(BW *bw)
{
	P *q = pdup(bw->cursor);

      up:
	while (pisnpara(q) && !pisbof(q) && (!within || !markb || q->byte > markb->byte))
		pprevl(q);
	pbop(q);
	if (q->byte != bw->cursor->byte) {
		pset(bw->cursor, q);
		prm(q);
		return 0;
	} else if (!pisbof(q)) {
		prgetc(q);
		goto up;
	} else {
		prm(q);
		return -1;
	}
}

int ueop(BW *bw)
{
	P *q = pdup(bw->cursor);

      up:
	while (pisnpara(q) && !piseof(q))
		pnextl(q);
	pbop(q);
	peop(q);
	if (q->byte != bw->cursor->byte) {
		pset(bw->cursor, q);
		prm(q);
		return 0;
	} else if (!piseof(q)) {
		pnextl(q);
		goto up;
	} else {
		prm(q);
		return -1;
	}
}

/* Wrap word.  If 'french' is set, only one space will be placed
 * after . ? or !
 */

void wrapword(P *p, long int indent, int french, unsigned char *indents)
{
	P *q;
	int rmf = 0;
	int c;
	long to = p->byte;

	/* Get indentation prefix from beginning of line */
/*
	if(!indents) {
		int f = 0;
		P *r = pdup(p);

		p_goto_bol(r);
		q = pdup(r);
		while(cpara(c = brc(q))) {
			if(!joe_isblank(c))
				f = 1;
			pgetc(q);
		}
		if(f) {
			indents = brs(r, q->byte-r->byte);
			rmf = 1;
			if(indents[0] == '/' && indents[1] == '*')
				indents[0] = ' ';
		}
		prm(r);
		prm(q);
	}
*/

	/* Get to beginning of word */
	while (!pisbol(p) && piscol(p) > indent && !joe_isblank(p->b->o.charmap, prgetc(p)))
		/* do nothing */;

	/* If we found the beginning of a word... */
	if (!pisbol(p) && piscol(p) > indent) {
		/* Move q to two (or one if 'french' is set) spaces after end of previous
		   word */
		q = pdup(p);
		while (!pisbol(q))
			if (!joe_isblank(p->b->o.charmap, (c = prgetc(q)))) {
				pgetc(q);
				if ((c == '.' || c == '?' || c == '!')
				    && q->byte != p->byte && !french)
					pgetc(q);
				break;
			}
		pgetc(p);

		/* Delete space between start of word and end of previous word */
		to -= p->byte - q->byte;
		bdel(q, p);
		prm(q);

		/* Move word to beginning of next line */
		binsc(p, '\n');
		++to;
		if (p->b->o.crlf)
			++to;
		pgetc(p);

		/* Indent to left margin */
		if (indents) {
			binss(p, indents);
			to += strlen((char *)indents);
		} else
			while (indent--) {
				binsc(p, ' ');
				++to;
			}

		if (rmf)
			joe_free(indents);
	}

	/* Move cursor back to original position */
	pfwrd(p, to - p->byte);
}

/* Reformat paragraph */

int uformat(BW *bw)
{
	long indent;
	unsigned char *indents;
	B *buf;
	P *b;
	long curoff;
	int c;
	P *p, *q;

	p = pdup(bw->cursor);
	p_goto_bol(p);

	/* Do nothing if we're not on a paragraph line */
	if (pisnpara(p)) {
		prm(p);
		return 0;
	}

	/* Move p to beginning of paragraph, bw->cursor to end of paragraph and
	 * set curoff to original cursor offset within the paragraph */
	pbop(p);
	curoff = bw->cursor->byte - p->byte;
	pset(bw->cursor, p);
	peop(bw->cursor);

	/* Ensure that paragraph ends on a beginning of a line */
	if (!pisbol(bw->cursor))
		binsc(bw->cursor, '\n'), pgetc(bw->cursor);

	/* Record indentation of second line of paragraph, of first line if there
	 * is only one line */
	q = pdup(p);
	pnextl(q);
	if (q->line != bw->cursor->line) {
		P *r = pdup(q);

		indent = nindent(q);
		pcol(r, indent);
		indents = brs(q, r->byte - q->byte);
		prm(r);
	} else {
		P *r = pdup(p);

		indent = nindent(p);
		pcol(r, indent);
		indents = brs(p, r->byte - p->byte);
		prm(r);
	}
	prm(q);

	/* But if the left margin is greater, we use that instead */
	if (bw->o.lmargin > indent)
		indent = bw->o.lmargin;

	/* Cut paragraph into new buffer */
	
	/* New buffer needs to inherit UTF-8 and CR-LF options */
	buf = bcpy(p, bw->cursor);
	buf->o.crlf = p->b->o.crlf;
	buf->o.charmap = p->b->o.charmap;
	bdel(p, bw->cursor);

	/* text is in buffer.  insert it at cursor */

	/* Do first line */
	b = pdup(buf->bof);

	while (!piseof(b)) {
		/* Set cursor position if we're at original offset */
		if (b->byte == curoff)
			pset(bw->cursor, p);

		/* Get character from buffer */
		c = pgetc(b);

		/* Stop if we found end of line */
		if (c == '\n') {
			prgetc(b);
			break;
		}

		/* Stop if we found white-space followed by end of line */
		if (joe_isblank(b->b->o.charmap, c) && piseolblank(b))
			break;

		/* Insert character, advance pointer */
		binsc(p, c);
		pgetc(p);

		/* Do word wrap if we reach right margin */
		if (piscol(p) > bw->o.rmargin && !joe_isblank(p->b->o.charmap,c)) {
			wrapword(p, indent, bw->o.french, indents);
			break;
		}
	}

	/* Do rest */

	while (!piseof(b)) {
		c = brch(b);
		if (joe_isblank(b->b->o.charmap,c) || c == '\n') {
			int f = 0;
			P *d;
			int g;

			/* Set f if there are two spaces after . ? or ! instead of one */
			/* (What is c was '\n'?) */
			d=pdup(b);
			g=prgetc(d);
			if (g=='.' || g=='?' || g=='!') {
				pset(d,b);
				pgetc(d);
				if (joe_isspace(bw->b->o.charmap,brch(d)))
					f = 1;
			}
			prm(d);
			
			/* Skip past the whitespace.  Skip over indentations */
		      loop:
			
			c = brch(b);
			if (c == '\n') {
				if (b->byte == curoff)
					pset(bw->cursor, p);

				pgetc(b);
				while (cpara(c=brch(b))) {
					if (b->byte == curoff)
						pset(bw->cursor, p);
					pgetc(b);
				}
			}

			if (joe_isblank(b->b->o.charmap,c)) {
				if(b->byte == curoff)
					pset(bw->cursor, p);
				pgetc(b);
				goto loop;
			}

			/* Insert proper amount of whitespace */
			if (!piseof(b)) {
				if (f && !bw->o.french)
					binsc(p, ' '), pgetc(p);
				binsc(p, ' ');
				pgetc(p);
			}
		} else {
			/* Insert characters of word and wrap if necessary */
			if (b->byte == curoff)
				pset(bw->cursor, p);

			binsc(p, pgetc(b));
			pgetc(p);
			if (piscol(p) > bw->o.rmargin)
				wrapword(p, indent, bw->o.french, indents);
		}
	}

	binsc(p, '\n');
	prm(p);
	brm(buf);
	joe_free(indents);
	return 0;
}

/* Format entire block */

extern int lightoff;

int ufmtblk(BW *bw)
{
	if (markv(1) && bw->cursor->byte >= markb->byte && bw->cursor->byte <= markk->byte) {
		markk->end = 1;
		utomarkk(bw);
		within = 1;
		do {
			ubop(bw), uformat(bw);
		} while (bw->cursor->byte > markb->byte);
		within = 0;
		markk->end = 0;
		if (lightoff)
			unmark(bw);
		return 0;
	} else
		return uformat(bw);
}
