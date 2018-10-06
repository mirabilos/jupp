/*
 *	Basic user edit functions
 *	Copyright
 * 		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#include "config.h"
#include "types.h"

__RCSID("$MirOS: contrib/code/jupp/uedit.c,v 1.36 2018/08/10 02:53:45 tg Exp $");

#include <string.h>

#ifdef HAVE_BSD_STRING_H
#include <bsd/string.h>
#endif

#include "b.h"
#include "bw.h"
#include "macro.h"
#include "main.h"
#include "pw.h"
#include "qw.h"
#include "scrn.h"
#include "ublock.h"
#include "uedit.h"
#include "uformat.h"
#include "umath.h"
#include "utils.h"
#include "vs.h"
#include "charmap.h"
#include "w.h"

/***************/
/* Global options */
int pgamnt = -1;		/* No. of PgUp/PgDn lines to keep */

/******** i don't like global var ******/

/*
 * Move cursor to beginning of line
 */
int u_goto_bol(BW *bw)
{
	if (bw->o.hex) {
		pbkwd(bw->cursor,bw->cursor->byte%16);
	} else {
		p_goto_bol(bw->cursor);
	}
	return 0;
}

/*
 * Move cursor to first non-whitespace character, unless it is
 * already there, in which case move it to beginning of line
 */
int uhome(BW *bw)
{
	P *p;

	if (bw->o.hex) {
		return u_goto_bol(bw);
	}

	p = pdup(bw->cursor);

	if (bw->o.indentfirst) {
		if ((bw->o.smarthome) && (piscol(p) > pisindent(p))) {
			p_goto_bol(p);
			while (joe_isblank(p->b->o.charmap,brc(p)))
				pgetc(p);
		} else
			p_goto_bol(p);
	} else {
		if (bw->o.smarthome && piscol(p)==0 && pisindent(p)) {
			while (joe_isblank(p->b->o.charmap,brc(p)))
				pgetc(p);
		} else
			p_goto_bol(p);
	}

	pset(bw->cursor, p);
	prm(p);
	return 0;
}

/*
 * Move cursor to end of line
 */
int u_goto_eol(BW *bw)
{
	if (bw->o.hex) {
		if (bw->cursor->byte + 15 - bw->cursor->byte%16 > bw->b->eof->byte)
			pset(bw->cursor,bw->b->eof);
		else
			pfwrd(bw->cursor, 15 - bw->cursor->byte%16);
	} else
		p_goto_eol(bw->cursor);
	return 0;
}

/*
 * Move cursor to beginning of file
 */
int u_goto_bof(BW *bw)
{
	p_goto_bof(bw->cursor);
	return 0;
}

/*
 * Move cursor to end of file
 */
int u_goto_eof(BW *bw)
{
	p_goto_eof(bw->cursor);
	return 0;
}

/*
 * Move cursor left
 */
int u_goto_left(BW *bw)
{
	if (bw->o.hex) {
		if (prgetb(bw->cursor) != NO_MORE_DATA) {
			return 0;
		} else {
			return -1;
		}
	}
	if (bw->o.picture) {
		if (bw->cursor->xcol) {
			--bw->cursor->xcol;
			pcol(bw->cursor,bw->cursor->xcol);
			return 0;
		}
	} else {
		/* Have to do ECHKXCOL here because of picture mode */
		if (bw->cursor->xcol != piscol(bw->cursor)) {
			bw->cursor->xcol = piscol(bw->cursor);
			return 0;
		} else if (prgetc(bw->cursor) != NO_MORE_DATA) {
			bw->cursor->xcol = piscol(bw->cursor);
			return 0;
		}
	}
	return -1;
}

/*
 * Move cursor right
 */
int u_goto_right(BW *bw)
{
	if (bw->o.hex) {
		if (pgetb(bw->cursor) != NO_MORE_DATA) {
			return 0;
		} else {
			return -1;
		}
	}
	if (bw->o.picture) {
		++bw->cursor->xcol;
		pcol(bw->cursor,bw->cursor->xcol);
		return 0;
	} else {
		int rtn;
		if (pgetc(bw->cursor) != NO_MORE_DATA) {
			bw->cursor->xcol = piscol(bw->cursor);
			rtn = 0;
		} else {
			rtn = -1;
		}
		/* Have to do EFIXXCOL here because of picture mode */
		if (bw->cursor->xcol != piscol(bw->cursor))
			bw->cursor->xcol = piscol(bw->cursor);
		return rtn;
	}
}

/*
 * Move cursor to beginning of previous word or if there isn't
 * previous word then go to beginning of the file
 *
 * WORD is a sequence non-white-space characters
 */
int u_goto_prev(BW *bw)
{
	P *p = pdup(bw->cursor);
	struct charmap *map=bw->b->o.charmap;
	int c = prgetc(p);

	if (joe_isalnux(map,c)) {
		while (joe_isalnux(map,(c=prgetc(p))))
			/* Do nothing */;
		if (c != NO_MORE_DATA)
			pgetc(p);
	} else if (joe_isspace(map,c) || joe_ispunct(map,c)) {
		while ((c=prgetc(p)), (joe_isspace(map,c) || joe_ispunct(map,c)))
			/* Do nothing */;
		while(joe_isalnux(map,(c=prgetc(p))))
			/* Do nothing */;
		if (c != NO_MORE_DATA)
			pgetc(p);
	}
/*
	if (p->byte == bw->cursor->byte) {
		prm(p);
		return -1;
	}
*/
	pset(bw->cursor, p);
	prm(p);
	return 0;
}

/*
 * Move cursor to end of next word or if there isn't
 * next word then go to end of the file
 *
 * WORD is a sequence non-white-space characters
 */
int u_goto_next(BW *bw)
{
	P *p = pdup(bw->cursor);
	struct charmap *map=bw->b->o.charmap;
	int c = brch(p);
	int rtn = -1;

	if (joe_isalnux(map,c)) {
		rtn = 0;
		while (joe_isalnux(map,(c = brch(p))))
			pgetc(p);
	} else if (joe_isspace(map,c) || joe_ispunct(map,c)) {
		while (joe_isspace(map, (c = brch(p))) || joe_ispunct(map,c))
			pgetc(p);
		while (joe_isalnux(map,(c = brch(p)))) {
			rtn = 0;
			pgetc(p);
		}
	} else
		pgetc(p);
	pset(bw->cursor, p);
	prm(p);
	return rtn;
}

static P *pboi(P *p)
{
	p_goto_bol(p);
	while (joe_isblank(p->b->o.charmap,brch(p)))
		pgetc(p);
	return p;
}

static int pisedge(P *p)
{
	P *q;
	int c;

	if (pisbol(p))
		return -1;
	if (piseol(p))
		return 1;
	q = pdup(p);
	pboi(q);
	if (q->byte == p->byte)
		goto left;
	if (joe_isblank(p->b->o.charmap,(c = brch(p)))) {
		pset(q, p);
		if (joe_isblank(p->b->o.charmap,prgetc(q)))
			goto no;
		if (c == '\t')
			goto right;
		pset(q, p);
		pgetc(q);
		if (pgetc(q) == ' ')
			goto right;
		goto no;
	} else {
		pset(q, p);
		c = prgetc(q);
		if (c == '\t')
			goto left;
		if (c != ' ')
			goto no;
		if (prgetc(q) == ' ')
			goto left;
		goto no;
	}

      right:prm(q);
	return 1;
      left:prm(q);
	return -1;
      no:prm(q);
	return 0;
}

int upedge(BW *bw)
{
	if (prgetc(bw->cursor) == NO_MORE_DATA)
		return -1;
	while (pisedge(bw->cursor) != -1)
		prgetc(bw->cursor);
	return 0;
}

int unedge(BW *bw)
{
	if (pgetc(bw->cursor) == NO_MORE_DATA)
		return -1;
	while (pisedge(bw->cursor) != 1)
		pgetc(bw->cursor);
	return 0;
}

/* Move cursor to matching delimiter */

static int
utomatch_i(BW *bw, int dir)
{
	int d;
	int c;			/* character under cursor */
	int f;			/* character to find */
	P *p;
	int cnt = 0;		/* delimiter depth */

	switch (f = c = brch(bw->cursor)) {
	case '(':
		f = ')';
		dir = 1;
		break;
	case '[':
		f = ']';
		dir = 1;
		break;
	case '{':
		f = '}';
		dir = 1;
		break;
	case '<':
		f = '>';
		dir = 1;
		break;
	case ')':
		f = '(';
		dir = -1;
		break;
	case ']':
		f = '[';
		dir = -1;
		break;
	case '}':
		f = '{';
		dir = -1;
		break;
	case '>':
		f = '<';
		dir = -1;
		break;
	}

	p = pdup(bw->cursor);
	if (dir == 1) {
		while ((d = pgetc(p)) != NO_MORE_DATA) {
			if (d == f && f != c && !--cnt) {
				prgetc(p);
				goto match_found;
			} else if (d == c) {
				++cnt;
				if (f == c)
					c = NO_MORE_DATA;
			}
		}
	} else {
		while ((d = prgetc(p)) != NO_MORE_DATA) {
			if (d == f && !cnt--)
				goto match_found;
			else if (d == c)
				++cnt;
		}
	}
	if (/* CONSTCOND */ 0) {
 match_found:
		pset(bw->cursor, p);
	}
	prm(p);
	return ((d == NO_MORE_DATA) ? -1 : 0);
}

int utomatch(BW *bw)
{
	return (utomatch_i(bw, 1));
}

int urvmatch(BW *bw)
{
	return (utomatch_i(bw, -1));
}

/* Move cursor up */

int uuparw(BW *bw)
{
	if (bw->o.hex) {
		if (bw->cursor->byte<16)
			return -1;
		else {
			pbkwd(bw->cursor, 16);
			return 0;
		}
	}
	if (bw->cursor->line) {
		pprevl(bw->cursor);
		pcol(bw->cursor, bw->cursor->xcol);
		return 0;
	} else
		return -1;
}

/* Move cursor down */

int udnarw(BW *bw)
{
	if (bw->o.hex) {
		if (bw->cursor->byte+16 <= bw->b->eof->byte) {
			pfwrd(bw->cursor, 16);
			return 0;
		} else if (bw->cursor->byte != bw->b->eof->byte) {
			pset(bw->cursor, bw->b->eof);
			return 0;
		} else {
			return -1;
		}
	}
	if (bw->cursor->line != bw->b->eof->line) {
		pnextl(bw->cursor);
		pcol(bw->cursor, bw->cursor->xcol);
		return 0;
	} else if(bw->o.picture) {
		p_goto_eol(bw->cursor);
		binsc(bw->cursor,'\n');
		pgetc(bw->cursor);
		pcol(bw->cursor, bw->cursor->xcol);
		return 0;
	} else
		return -1;
}

/* Move cursor to top of window */

int utos(BW *bw)
{
	long col = bw->cursor->xcol;

	pset(bw->cursor, bw->top);
	pcol(bw->cursor, col);
	bw->cursor->xcol = col;
	return 0;
}

/* Move cursor to bottom of window */

int ubos(BW *bw)
{
	long col = bw->cursor->xcol;

	pline(bw->cursor, bw->top->line + bw->h - 1);
	pcol(bw->cursor, col);
	bw->cursor->xcol = col;
	return 0;
}

/* Scroll buffer window up n lines
 * If beginning of file is close, scrolls as much as it can
 * If beginning of file is on-screen, cursor jumps to beginning of file
 *
 * If flg is set: cursor stays fixed relative to screen edge
 * If flg is clr: cursor stays fixed on the buffer line
 */

void scrup(BW *bw, int n, int flg)
{
	int scrollamnt = 0;
	int cursoramnt = 0;
	int x;

	/* Decide number of lines we're really going to scroll */

	if (bw->o.hex) {
		if (bw->top->byte/16 >= n)
			scrollamnt = cursoramnt = n;
		else if (bw->top->byte/16)
			scrollamnt = cursoramnt = bw->top->byte/16;
		else if (flg)
			cursoramnt = bw->cursor->byte/16;
		else if (bw->cursor->byte/16 >= n)
			cursoramnt = n;
	} else {
		if (bw->top->line >= n)
			scrollamnt = cursoramnt = n;
		else if (bw->top->line)
			scrollamnt = cursoramnt = bw->top->line;
		else if (flg)
			cursoramnt = bw->cursor->line;
		else if (bw->cursor->line >= n)
			cursoramnt = n;
	}

	if (bw->o.hex) {
		/* Move top-of-window pointer */
		pbkwd(bw->top,scrollamnt*16);
		/* Move cursor */
		pbkwd(bw->cursor,cursoramnt*16);
		/* If window is on the screen, give (buffered) scrolling command */
		if (bw->parent->y != -1)
			nscrldn(bw->parent->t->t, bw->y, bw->y + bw->h, scrollamnt);
	} else {
		/* Move top-of-window pointer */
		for (x = 0; x != scrollamnt; ++x)
			pprevl(bw->top);
		p_goto_bol(bw->top);

		/* Move cursor */
		for (x = 0; x != cursoramnt; ++x)
			pprevl(bw->cursor);
		p_goto_bol(bw->cursor);
		pcol(bw->cursor, bw->cursor->xcol);

		/* If window is on the screen, give (buffered) scrolling command */
		if (bw->parent->y != -1)
			nscrldn(bw->parent->t->t, bw->y, bw->y + bw->h, scrollamnt);
	}
}

/* Scroll buffer window down n lines
 * If end of file is close, scrolls as much as possible
 * If end of file is on-screen, cursor jumps to end of file
 *
 * If flg is set: cursor stays fixed relative to screen edge
 * If flg is clr: cursor stays fixed on the buffer line
 */

void scrdn(BW *bw, int n, int flg)
{
	int scrollamnt = 0;
	int cursoramnt = 0;
	int x;

	/* How much we're really going to scroll... */
	if (bw->o.hex) {
		if (bw->top->b->eof->byte/16 < bw->top->byte/16 + bw->h) {
			cursoramnt = bw->top->b->eof->byte/16 - bw->cursor->byte/16;
			if (!flg && cursoramnt > n)
				cursoramnt = n;
		} else if (bw->top->b->eof->byte/16 - (bw->top->byte/16 + bw->h) >= n)
			cursoramnt = scrollamnt = n;
		else
			cursoramnt = scrollamnt = bw->top->b->eof->byte/16 - (bw->top->byte/16 + bw->h) + 1;
	} else {
		if (bw->top->b->eof->line < bw->top->line + bw->h) {
			cursoramnt = bw->top->b->eof->line - bw->cursor->line;
			if (!flg && cursoramnt > n)
				cursoramnt = n;
		} else if (bw->top->b->eof->line - (bw->top->line + bw->h) >= n)
			cursoramnt = scrollamnt = n;
		else
			cursoramnt = scrollamnt = bw->top->b->eof->line - (bw->top->line + bw->h) + 1;
	}

	if (bw->o.hex) {
		/* Move top-of-window pointer */
		pfwrd(bw->top,16*scrollamnt);
		/* Move cursor */
		pfwrd(bw->cursor,16*cursoramnt);
		/* If window is on screen, give (buffered) scrolling command to terminal */
		if (bw->parent->y != -1)
			nscrlup(bw->parent->t->t, bw->y, bw->y + bw->h, scrollamnt);
	} else {
		/* Move top-of-window pointer */
		for (x = 0; x != scrollamnt; ++x)
			pnextl(bw->top);

		/* Move cursor */
		for (x = 0; x != cursoramnt; ++x)
			pnextl(bw->cursor);
		pcol(bw->cursor, bw->cursor->xcol);

		/* If window is on screen, give (buffered) scrolling command to terminal */
		if (bw->parent->y != -1)
			nscrlup(bw->parent->t->t, bw->y, bw->y + bw->h, scrollamnt);
	}
}

/* Page up */

int upgup(BW *bw)
{
	bw = bw->parent->main->object.bw;
	if (bw->o.hex ? bw->cursor->byte < 16 : !bw->cursor->line)
		return -1;
	if (pgamnt < 0)
		scrup(bw, bw->h / 2 + bw->h % 2, 1);
	else if (pgamnt < bw->h)
		scrup(bw, bw->h - pgamnt, 1);
	else
		scrup(bw, 1, 1);
	return 0;
}

/* Page down */

int upgdn(BW *bw)
{
	bw = bw->parent->main->object.bw;
	if (bw->o.hex ? bw->cursor->byte/16 == bw->b->eof->byte/16 : bw->cursor->line == bw->b->eof->line)
		return -1;
	if (pgamnt < 0)
		scrdn(bw, bw->h / 2 + bw->h % 2, 1);
	else if (pgamnt < bw->h)
		scrdn(bw, bw->h - pgamnt, 1);
	else
		scrdn(bw, 1, 1);
	return 0;
}

/* Scroll by a single line.  The cursor moves with the scroll */

int uupslide(BW *bw)
{
	bw = bw->parent->main->object.bw;
	if (bw->o.hex ? bw->top->byte/16 : bw->top->line) {
		if (bw->o.hex ? bw->top->byte/16 + bw->h -1 != bw->cursor->byte/16 : bw->top->line + bw->h - 1 != bw->cursor->line)
			udnarw(bw);
		scrup(bw, 1, 0);
		return 0;
	} else
		return -1;
}

int udnslide(BW *bw)
{
	bw = bw->parent->main->object.bw;
	if (bw->o.hex ? bw->top->line/16 + bw->h <= bw->top->b->eof->byte/16 : bw->top->line + bw->h <= bw->top->b->eof->line) {
		if (bw->o.hex ? bw->top->byte/16 != bw->cursor->byte/16 : bw->top->line != bw->cursor->line)
			uuparw(bw);
		scrdn(bw, 1, 0);
		return 0;
	} else
		return -1;
}

/* Move cursor to specified line number */

static B *linehist = NULL;	/* History of previously entered line numbers */

static int doline(BW *bw, unsigned char *s, void *object, int *notify)
{
	long num = calcl(bw, s);

	if (notify)
		*notify = 1;
	vsrm(s);
	if (num >= 1 && !merrf) {
		int tmp = mid;

		if (num > bw->b->eof->line)
			num = bw->b->eof->line + 1;
		pline(bw->cursor, num - 1), bw->cursor->xcol = piscol(bw->cursor);
		mid = 1;
		dofollows();
		mid = tmp;
		return 0;
	} else {
		if (merrf)
			msgnw(bw->parent, merrt);
		else
			msgnw(bw->parent, UC "Invalid line number");
		return -1;
	}
}

int uline(BW *bw)
{
	if (wmkpw(bw->parent, UC "Go to line (^C to abort): ", &linehist, doline, NULL, NULL, NULL, NULL, NULL, locale_map))
		return 0;
	else
		return -1;
}

/* Move cursor to specified column number */

static B *colhist = NULL;	/* History of previously entered column numbers */

static int docol(BW *bw, unsigned char *s, void *object, int *notify)
{
	long num = calcl(bw, s);

	if (notify)
		*notify = 1;
	vsrm(s);
	if (num >= 1 && !merrf) {
		int tmp = mid;

		pcol(bw->cursor, num - 1), bw->cursor->xcol = piscol(bw->cursor);
		mid = 1;
		dofollows();
		mid = tmp;
		return 0;
	} else {
		if (merrf)
			msgnw(bw->parent, merrt);
		else
			msgnw(bw->parent, UC "Invalid column number");
		return -1;
	}
}

int ucol(BW *bw)
{
	if (wmkpw(bw->parent, UC "Go to column (^C to abort): ", &colhist, docol, NULL, NULL, NULL, NULL, NULL, locale_map))
		return 0;
	else
		return -1;
}

/* Move cursor to specified byte number */

static B *bytehist = NULL;	/* History of previously entered byte numbers */

static int dobyte(BW *bw, unsigned char *s, void *object, int *notify)
{
	long num = calcl(bw, s);

	if (notify)
		*notify = 1;
	vsrm(s);
	if (num >= 0 && !merrf) {
		int tmp = mid;

		pgoto(bw->cursor, num), bw->cursor->xcol = piscol(bw->cursor);
		mid = 1;
		dofollows();
		mid = tmp;
		return 0;
	} else {
		if (merrf)
			msgnw(bw->parent, merrt);
		else
			msgnw(bw->parent, UC "Invalid byte number");
		return -1;
	}
}

int ubyte(BW *bw)
{
	if (wmkpw(bw->parent, UC "Go to byte (^C to abort): ", &bytehist, dobyte, NULL, NULL, NULL, NULL, NULL, locale_map))
		return 0;
	else
		return -1;
}

/* Delete character under cursor
 * or write ^D to process if we're at end of file in a shell window
 */

int udelch(BW *bw)
{
	P *p;

	if (piseof(bw->cursor))
		return -1;
	pgetc(p = pdup(bw->cursor));
	bdel(bw->cursor, p);
	prm(p);
	return 0;
}

/* Backspace */

int ubacks(BW *bw, int k)
{
	/* Don't backspace when at beginning of line in prompt windows */
	if (bw->parent->watom->what == TYPETW || !pisbol(bw->cursor)) {
		int c;
		int indent;
		int col;
		int indwid;
		int wid;

		if (pisbof(bw->cursor))
			return -1;

		/* Indentation point of this line */
		indent = pisindent(bw->cursor);

		/* Column position of cursor */
		col = piscol(bw->cursor);

		/* Indentation step in columns */
		if (bw->o.indentc=='\t')
			wid = bw->o.tab;
		else
			wid = 1;

		indwid = (bw->o.istep*wid);

		/* Smart backspace when: cursor is at indentation point, indentation point
		   is a multiple of indentation width, we're not at beginning of line,
		   'smarthome' option is enabled, and indentation is purely made out of
		   indent characters (or purify indents is enabled). */

		/* Ignore purify for backspace */
		if (col == indent && (col%indwid)==0 && col!=0 && bw->o.smartbacks) {
			P *p;

			/* Delete all indentation */
			p = pdup(bw->cursor);
			p_goto_bol(p);
			bdel(p,bw->cursor);
			prm(p);

			/* Indent to new position */
			pfill(bw->cursor,col-indwid,bw->o.indentc);
		} else if (col<indent && !pisbol(bw->cursor) && bw->o.smartbacks) {
			/* We're before indent point: delete indwid worth of space but do not
			   cross line boundary.  We could probably replace the above with this. */
			int cw=0;
			P *p = pdup(bw->cursor);
			do {
				c = prgetc(bw->cursor);
				if(c=='\t') cw += bw->o.tab;
				else cw += 1;
				bdel(bw->cursor, p);
			} while(!pisbol(bw->cursor) && cw<indwid);
			prm(p);
		} else {
			/* Regular backspace */
			P *p = pdup(bw->cursor);
			if ((c = prgetc(bw->cursor)) != NO_MORE_DATA)
				if (!bw->o.overtype || c == '\t' || pisbol(p) || piseol(p))
					bdel(bw->cursor, p);
			prm(p);
		}
		return 0;
	} else
		return -1;
}

/*
 * Delete sequence of characters (alphabetic, numeric) or (white-space)
 *	if cursor is on the white-space it will delete all white-spaces
 *		until alphanumeric character
 *      if cursor is on the alphanumeric it will delete all alphanumeric
 *		characters until character that is not alphanumeric
 */
int u_word_delete(BW *bw)
{
	P *p = pdup(bw->cursor);
	struct charmap *map=bw->b->o.charmap;
	int c = brch(p);

	if (joe_isalnux(map,c))
		while (joe_isalnux(map,(c = brch(p))))
			pgetc(p);
	else if (joe_isspace(map,c))
		while (joe_isspace(map,(c = brch(p))))
			pgetc(p);
	else
		pgetc(p);

	if (p->byte == bw->cursor->byte) {
		prm(p);
		return -1;
	}
	bdel(bw->cursor, p);
	prm(p);
	return 0;
}

/* Delete from cursor to beginning of word it's in or immediately after,
 * to start of whitespace, or a single character
 */

int ubackw(BW *bw)
{
	P *p = pdup(bw->cursor);
	int c = prgetc(bw->cursor);
	struct charmap *map=bw->b->o.charmap;

	if (joe_isalnux(map,c)) {
		while (joe_isalnux(map,(c = prgetc(bw->cursor))))
			/* do nothing */;
		if (c != NO_MORE_DATA)
			pgetc(bw->cursor);
	} else if (joe_isspace(map,c)) {
		while (joe_isspace(map,(c = prgetc(bw->cursor))))
			/* do nothing */;
		if (c != NO_MORE_DATA)
			pgetc(bw->cursor);
	}
	if (bw->cursor->byte == p->byte) {
		prm(p);
		return -1;
	}
	bdel(bw->cursor, p);
	prm(p);
	return 0;
}

/* Delete from cursor to end of line, or if there's nothing to delete,
 * delete the line-break
 */

int udelel(BW *bw)
{
	P *p = p_goto_eol(pdup(bw->cursor));

	if (bw->cursor->byte == p->byte) {
		prm(p);
		return udelch(bw);
	} else
		bdel(bw->cursor, p);
	prm(p);
	return 0;
}

/* Delete to beginning of line, or if there's nothing to delete,
 * delete the line-break
 */

int udelbl(BW *bw)
{
	P *p = p_goto_bol(pdup(bw->cursor));

	if (p->byte == bw->cursor->byte) {
		prm(p);
		return ubacks(bw, 8);	/* The 8 goes to the process if we're at EOF of shell window */
	} else
		bdel(p, bw->cursor);
	prm(p);
	return 0;
}

/* Delete entire line */

int udelln(BW *bw)
{
	P *p = pdup(bw->cursor);

	p_goto_bol(bw->cursor);
	pnextl(p);
	if (bw->cursor->byte == p->byte) {
		prm(p);
		return -1;
	}
	bdel(bw->cursor, p);
	prm(p);
	return 0;
}

/* Insert a space */

int uinsc(BW *bw)
{
	binsc(bw->cursor, ' ');
	return 0;
}

/* Move p backwards to first non-blank line and return its indentation */

static int
find_indent(P *p)
{
	int x;
	for (x=0; x != 10; ++x) {
		if (!pprevl(p)) return -1;
		p_goto_bol(p);
		if (!pisblank(p)) break;
	}
	if (x==10)
		return -1;
	else
		return pisindent(p);
}

/* Type a character into the buffer (deal with left margin, overtype mode and
 * word-wrap), if cursor is at end of shell window buffer, just send character
 * to process.
 */

struct utf8_sm utype_utf8_sm;

int utypebw_raw(BW *bw, int k, int no_decode)
{
	struct charmap *map=bw->b->o.charmap;

	/* Hex mode overtype is real simple */
	if (bw->o.hex && bw->o.overtype) {
		P *p;
		unsigned char c = k;
		binsm(bw->cursor, &c, 1);
		pgetb(bw->cursor);
		if (piseof(bw->cursor))
			return 0;
		pgetb(p = pdup(bw->cursor));
		bdel(bw->cursor, p);
		prm(p);
		return 0;
	}

	if (k == '\t' && bw->o.overtype && !piseol(bw->cursor)) { /* TAB in overtype mode is supposed to be just cursor motion */
		int col = bw->cursor->xcol;		/* Current cursor column */
		col = col + bw->o.tab - (col%bw->o.tab);/* Move to next tab stop */
		pcol(bw->cursor,col);			/* Try to position cursor there */
		if (!bw->o.picture && piseol(bw->cursor) && piscol(bw->cursor)<col) {	/* We moved past end of line, insert a tab (unless in picture mode) */
			if (bw->o.spaces)
				pfill(bw->cursor,col,' ');
			else
				pfill(bw->cursor,col,'\t');
		}
		bw->cursor->xcol = col;			/* Put cursor there even if we can't really go there */
	} else if (k == '\t' && bw->o.smartbacks && bw->o.autoindent && pisindent(bw->cursor)>=piscol(bw->cursor)) {
		P *p = pdup(bw->cursor);
		int n = find_indent(p);
		if (n != -1 && pisindent(bw->cursor)==piscol(bw->cursor) && n > pisindent(bw->cursor)) {
			if (!pisbol(bw->cursor))
				udelbl(bw);
			while (joe_isspace(map,(k = pgetc(p))) && k != '\n') {
				binsc(bw->cursor, k);
				pgetc(bw->cursor);
			}
		} else {
			int x;
			for (x=0;x<bw->o.istep;++x) {
				binsc(bw->cursor,bw->o.indentc);
				pgetc(bw->cursor);
			}
		}
		bw->cursor->xcol = piscol(bw->cursor);
		prm (p);
	} else if (k == '\t' && bw->o.spaces) {
		long n;

		if (bw->o.picture)
			n = bw->cursor->xcol;
		else
			n = piscol(bw->cursor);

		utype_utf8_sm.state = 0;
		utype_utf8_sm.ptr = 0;

		n = bw->o.tab - n % bw->o.tab;
		while (n--)
			utypebw_raw(bw, ' ', 0);
	} else {
		int simple;
		int x;

		/* Picture mode */
		if (bw->o.picture && bw->cursor->xcol!=piscol(bw->cursor))
			pfill(bw->cursor,bw->cursor->xcol,' '); /* Why no tabs? */

		/* UTF8 decoder */
		if(locale_map->type && !no_decode) {
			int utf8_char = utf8_decode(&utype_utf8_sm,k);

			if(utf8_char >= 0)
				k = utf8_char;
			else
				return 0;
		}

		simple = 1;

		if (pisblank(bw->cursor))
			while (piscol(bw->cursor) < bw->o.lmargin) {
				binsc(bw->cursor, ' ');
				pgetc(bw->cursor);
			}

		if (no_decode == 2) {
			unsigned char ch = k;

			binsm(bw->cursor, &ch, 1);
			if (!bw->b->o.charmap->type)
				no_decode = 1;
		} else {
			if (!no_decode) {
				if(locale_map->type && !bw->b->o.charmap->type) {
					unsigned char buf[10];
					utf8_encode(buf,k);
					k = from_utf8(bw->b->o.charmap,buf);
				} else if(!locale_map->type && bw->b->o.charmap->type) {
					unsigned char buf[10];
					to_utf8(locale_map,buf,k);
					k = utf8_decode_string(buf);
				}
			}

			binsc(bw->cursor, k);
		}

		/* We need x position before we move cursor */
		x = piscol(bw->cursor) - bw->offset;
		pgetc(bw->cursor);

		/* Tabs are weird here... */
		if (bw->o.overtype && !piseol(bw->cursor) && k != '\t')
			udelch(bw);

		/* Not sure if we're in right position for wordwrap when we're in overtype mode */
		if (bw->o.wordwrap && piscol(bw->cursor) > bw->o.rmargin && !joe_isblank(map,k)) {
			wrapword(bw->cursor, (long) bw->o.lmargin, bw->o.french, NULL);
			simple = 0;
		}

		bw->cursor->xcol = piscol(bw->cursor);
		if (x < 0 || x >= bw->w)
			simple = 0;
		if (bw->cursor->line < bw->top->line || bw->cursor->line >= bw->top->line + bw->h)
			simple = 0;
		if (simple && bw->parent->t->t->sary[bw->y + bw->cursor->line - bw->top->line])
			simple = 0;
		else if (simple)
			switch (k) {
			case ' ':
				if (bw->o.vispace)
					/* FALLTHROUGH */
			case '\t':
			case '\n':
				  simple = 0;
				break;
			}
		if (simple && !curmacro) {
			int atr = 0;
			SCRN *t = bw->parent->t->t;
			int y = bw->y + bw->cursor->line - bw->top->line;
			int *screen = t->scrn + y * t->co;
			int *attr = t->attr + y * t->co;
			x += bw->x;

			if (!bw->parent->t->t->updtab[bw->y + bw->cursor->line - bw->top->line] &&
			    piseol(bw->cursor) && !bw->o.highlight)
				t->updtab[y] = 0;
			if (markb &&
			    markk &&
			    markb->b == bw->b &&
			    markk->b == bw->b &&
			   ((!square && bw->cursor->byte >= markb->byte && bw->cursor->byte < markk->byte) ||
			    ( square && bw->cursor->line >= markb->line && bw->cursor->line <= markk->line && piscol(bw->cursor) >= markb->xcol && piscol(bw->cursor) < markk->xcol)))
				atr = INVERSE;
			outatr(bw->b->o.charmap, t, screen + x, attr + x, x, y, no_decode == 2 ? 0xFFFD : k, atr);
		}
	}
	return 0;
}

int utypebw(jobject jO, int k)
{
	return utypebw_raw(jO.bw, k, 0);
}

/* Quoting */

static B *unicodehist = NULL;	/* History of previously entered UCS characters */

static int dounicode(BW *bw, unsigned char *s, void *object, int *notify)
{
	int num;

	num = ustolb(s, NULL, 0, 0x10FFFF, USTOL_HEX | USTOL_TRIM | USTOL_EOS);
	if (notify)
		*notify = 1;
	vsrm(s);
	if (bw->b->o.charmap->type)
		utypebw_raw(bw, num, 1);
	else {
		unsigned char buf[8];
		int x;

		utf8_encode(buf,num);
		for(x=0;buf[x];++x)
			utypebw_raw(bw, buf[x], 1);
	}
	bw->cursor->xcol = piscol(bw->cursor);
	return 0;
}

static void
doquote0(BW *bw, int c, int meta)
{
	if (c == '?')
		c = 127;
	else if ((c >= 0x40 && c <= 0x5F) || (c >= 'a' && c <= 'z'))
		c &= 0x1F;
	c |= meta;
	utypebw_raw(bw, c, 1);
	bw->cursor->xcol = piscol(bw->cursor);
}

int quotestate;
int quoteval;

static int
doquote(BW *bw, int c, void *object, int *notify)
{
	unsigned char buf[40];

	if (c < 0 || c >= 256) {
		nungetc(c);
		return -1;
	}
	switch (quotestate) {
	case 0:
		if (c >= '0' && c <= '9') {
			quoteval = c - '0';
			quotestate = 1;
			joe_snprintf_1((char *)buf, sizeof(buf), "ASCII %c--", c);
			if (!mkqwna(bw->parent, sz(buf), doquote, NULL, NULL, notify))
				return -1;
			else
				return 0;
		} else if ((c | 0x20) == 'u') {
			if (bw->b->o.charmap->type)
				goto unopoo;
 uhex_uni:
			if (!wmkpw(bw->parent, UC "UCS (ISO-10646) character in hex (^C to abort): ", &unicodehist, dounicode,
				   NULL, NULL, NULL, NULL, NULL, locale_map))
				return 0;
			else
				return -1;
		} else if ((c | 0x20) == 'r') {
			if (!bw->b->o.charmap->type)
				goto unopoo;
 uhex_raw:
			quotestate = 3;
			if (!mkqwna(bw->parent, sc("ASCII 0x--"), doquote, NULL, NULL, notify))
				return -1;
			else
				return 0;
		} else if ((c | 0x20) == 'x') {
			if (bw->b->o.charmap->type)
				goto uhex_uni;
			else
				goto uhex_raw;
		} else if ((c | 0x20) == 'o') {
			quotestate = 5;
			if (!mkqwna(bw->parent, sc("ASCII 0---"), doquote, NULL, NULL, notify))
				return -1;
			else
				return 0;
		} else {
 unopoo:
			doquote0(bw, c, 0);
		}
		break;
	case 1:
		if (c >= '0' && c <= '9') {
			joe_snprintf_2((char *)buf, sizeof(buf), "ASCII %c%c-", quoteval + '0', c);
			quoteval = quoteval * 10 + c - '0';
			quotestate = 2;
			if (!mkqwna(bw->parent, sz(buf), doquote, NULL, NULL, notify))
				return -1;
			else
				return 0;
		}
		break;
	case 2:
		if (c >= '0' && c <= '9') {
			quoteval = quoteval * 10 + c - '0';
			utypebw_raw(bw, quoteval, 1);
			bw->cursor->xcol = piscol(bw->cursor);
		}
		break;
	case 3:
		if (c >= '0' && c <= '9') {
			quoteval = c - '0';
 uhex_3:
			joe_snprintf_1((char *)buf, sizeof(buf), "ASCII 0x%c-", c);
			quotestate = 4;
			if (!mkqwna(bw->parent, sz(buf), doquote, NULL, NULL, notify))
				return -1;
			else
				return 0;
		}
		c &= ~0x20;
		if (c >= 'A' && c <= 'F') {
			quoteval = c - 'A' + 10;
			goto uhex_3;
		}
		break;
	case 4:
		if (c >= '0' && c <= '9') {
			quoteval = quoteval * 16 + c - '0';
 u4out:
			utypebw_raw(bw, quoteval, 2);
			bw->cursor->xcol = piscol(bw->cursor);
		} else if (c >= 'a' && c <= 'f') {
			quoteval = quoteval * 16 + c - 'a' + 10;
			goto u4out;
		} else if (c >= 'A' && c <= 'F') {
			quoteval = quoteval * 16 + c - 'A' + 10;
			goto u4out;
		}
		break;
	case 5:
		if (c >= '0' && c <= '7') {
			joe_snprintf_1((char *)buf, sizeof(buf), "ASCII 0%c--", c);
			quoteval = c - '0';
			quotestate = 6;
			if (!mkqwna(bw->parent, sz(buf), doquote, NULL, NULL, notify))
				return -1;
			else
				return 0;
		}
		break;
	case 6:
		if (c >= '0' && c <= '7') {
			joe_snprintf_2((char *)buf, sizeof(buf), "ASCII 0%c%c-", quoteval + '0', c);
			quoteval = quoteval * 8 + c - '0';
			quotestate = 7;
			if (!mkqwna(bw->parent, sz(buf), doquote, NULL, NULL, notify))
				return -1;
			else
				return 0;
		}
		break;
	case 7:
		if (c >= '0' && c <= '7') {
			quoteval = quoteval * 8 + c - '0';
			utypebw_raw(bw, quoteval, 1);
			bw->cursor->xcol = piscol(bw->cursor);
		}
		break;
	}
	if (notify)
		*notify = 1;
	return 0;
}

static char uquote_txt[] =
    "Ctrl- (or 0-9 for dec. o for octal, x hex, u UCS)";
int
uquote(BW *bw)
{
	quotestate = 0;
	if (bw->b->o.charmap->type) {
		uquote_txt[36] = 'r';
		uquote_txt[43] = 'x';
	} else {
		uquote_txt[36] = 'x';
		uquote_txt[43] = 'u';
	}
	return (mkqwna(bw->parent, sc(uquote_txt),
	    doquote, NULL, NULL, NULL) ? 0 : -1);
}

static int doquote9(BW *bw, int c, void *object, int *notify)
{
	if (notify)
		*notify = 1;
	doquote0(bw, c, 128);
	return 0;
}

static int doquote8(BW *bw, int c, void *object, int *notify)
{
	if (c == '`') {
		if (mkqwna(bw->parent, sc("Meta-Ctrl-"), doquote9, NULL, NULL, notify))
			return 0;
		else
			return -1;
	}
	if (notify)
		*notify = 1;
	c |= 128;
	utypebw_raw(bw, c, 1);
	bw->cursor->xcol = piscol(bw->cursor);
	return 0;
}

int uquote8(BW *bw)
{
	if (mkqwna(bw->parent, sc("Meta-"), doquote8, NULL, NULL, NULL))
		return 0;
	else
		return -1;
}

extern const unsigned char srchstr[];

static int doctrl(BW *bw, int c, void *object, int *notify)
{
	int org = bw->o.overtype;

	if (notify)
		*notify = 1;
	bw->o.overtype = 0;
	if (bw->parent->huh == srchstr && c == '\n') {
		utypebw_raw(bw, '\\', 0);
		utypebw_raw(bw, 'n', 0);
	} else
		utypebw_raw(bw, c, 1);
	bw->o.overtype = org;
	bw->cursor->xcol = piscol(bw->cursor);
	return 0;
}

int uctrl(BW *bw)
{
	if (mkqwna(bw->parent, sc("Quote"), doctrl, NULL, NULL, NULL))
		return 0;
	else
		return -1;
}

/* User hit Return.  Deal with autoindent.
 */

int rtntw(jobject jO)
{
	BW *bw = jO.bw;

	if (bw->o.overtype) {
		p_goto_eol(bw->cursor);
		if (piseof(bw->cursor))
			binsc(bw->cursor, '\n');
		pgetc(bw->cursor);
		bw->cursor->xcol = piscol(bw->cursor);
	} else {
		P *p = pdup(bw->cursor);
		unsigned char c;

		binsc(bw->cursor, '\n'), pgetc(bw->cursor);
		/* Suppress autoindent if we're on a space or tab... */
		if (bw->o.autoindent && (brch(bw->cursor)!=' ' && brch(bw->cursor)!='\t')) {
			p_goto_bol(p);
			while (joe_isspace(bw->b->o.charmap,(c = pgetc(p))) && c != '\n') {
				binsc(bw->cursor, c);
				pgetc(bw->cursor);
			}
		}
		prm(p);
		bw->cursor->xcol = piscol(bw->cursor);
	}
	return 0;
}

/* Open a line */

int uopen(BW *bw)
{
	binsc(bw->cursor,'\n');
	return 0;
}

/* Set book-mark */

static int dosetmark(BW *bw, int c, void *object, int *notify)
{
	if (notify)
		*notify = 1;
	if (c >= '0' && c <= ':') {
		pdupown(bw->cursor, bw->b->marks + c - '0');
		poffline(bw->b->marks[c - '0']);
		if (c!=':') {
			joe_snprintf_1((char *)msgbuf, JOE_MSGBUFSIZE, "Mark %d set", c - '0');
			msgnw(bw->parent, msgbuf);
		}
		return 0;
	} else {
		nungetc(c);
		return -1;
	}
}

int usetmark(BW *bw, int c)
{
	if (c >= '0' && c <= ':')
		return dosetmark(bw, c, NULL, NULL);
	else if (mkqwna(bw->parent, sc("Set mark (0-9):"), dosetmark, NULL, NULL, NULL))
		return 0;
	else
		return -1;
}

/* Goto book-mark */

static int dogomark(BW *bw, int c, void *object, int *notify)
{
	if (notify)
		*notify = 1;
	if (c >= '0' && c <= ':')
		if (bw->b->marks[c - '0']) {
			pset(bw->cursor, bw->b->marks[c - '0']);
			bw->cursor->xcol = piscol(bw->cursor);
			return 0;
		} else {
			joe_snprintf_1((char *)msgbuf, JOE_MSGBUFSIZE, "Mark %d not set", c - '0');
			msgnw(bw->parent, msgbuf);
			return -1;
	} else {
		nungetc(c);
		return -1;
	}
}

int ugomark(BW *bw, int c)
{
	if (c >= '0' && c <= '9')
		return dogomark(bw, c, NULL, NULL);
	else if (mkqwna(bw->parent, sc("Goto bookmark (0-9):"), dogomark, NULL, NULL, NULL))
		return 0;
	else
		return -1;
}

/* Goto next instance of character */

static int dobkwdc;

static int dofwrdc(BW *bw, int k, void *object, int *notify)
{
	int c;
	P *q;

	if (notify)
		*notify = 1;
	if (k < 0 || k >= 256) {
		nungetc(k);
		return -1;
	}
	q = pdup(bw->cursor);
	if (dobkwdc) {
		while ((c = prgetc(q)) != NO_MORE_DATA)
			if (c == k)
				break;
	} else {
		while ((c = pgetc(q)) != NO_MORE_DATA)
			if (c == k)
				break;
	}
	if (c == NO_MORE_DATA) {
		msgnw(bw->parent, UC "Not found");
		prm(q);
		return -1;
	} else {
		pset(bw->cursor, q);
		bw->cursor->xcol = piscol(bw->cursor);
		prm(q);
		return 0;
	}
}

int ufwrdc(BW *bw, int k)
{
	dobkwdc = 0;
	if (k >= 0 && k < 256)
		return dofwrdc(bw, k, NULL, NULL);
	else if (mkqw(bw->parent, sc("Fwrd to char: "), dofwrdc, NULL, NULL, NULL))
		return 0;
	else
		return -1;
}

int ubkwdc(BW *bw, int k)
{
	dobkwdc = 1;
	if (k >= 0 && k < 256)
		return dofwrdc(bw, k, NULL, NULL);
	else if (mkqw(bw->parent, sc("Bkwd to char: "), dofwrdc, NULL, NULL, NULL))
		return 0;
	else
		return -1;
}

/* Display a message */

static int domsg(BASE *b, unsigned char *s, void *object, int *notify)
{
	if (notify)
		*notify = 1;
	strlcpy((char *)msgbuf, (char *)s, JOE_MSGBUFSIZE);
	vsrm(s);
	msgnw(b->parent, *msgbuf ? msgbuf : NULL);
	return 0;
}

int umsg(BASE *b)
{
	if (wmkpw(b->parent, UC "Msg (^C to abort): ", NULL, domsg, NULL, NULL, NULL, NULL, NULL, locale_map))
		return 0;
	else
		return -1;
}

/* Insert text */

static int dotxt(BW *bw, unsigned char *s, void *object, int *notify)
{
	int x;

	if (notify)
		*notify = 1;
	for (x = 0; x != sLEN(s); ++x)
		utypebw_raw(bw, s[x], 0);
	vsrm(s);
	return 0;
}

int utxt(BW *bw)
{
	if (wmkpw(bw->parent, UC "Insert (^C to abort): ", NULL, dotxt, NULL, NULL, utypebw, NULL, NULL, bw->b->o.charmap))
		return 0;
	else
		return -1;
}
