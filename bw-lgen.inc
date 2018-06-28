/*
 *	Edit buffer single line update core loop (twice)
 *	Copyright © 1992 Joseph H. Allen
 *	Copyright © 2018 mirabilos
 *
 *	This file is part of jupp (Joe’s Own Editor fork)
 */

#ifdef LGEN_Q
__IDSTRING(rcsid_bw_lgen_inc, "$MirOS: contrib/code/jupp/bw-lgen.inc,v 1.5 2018/06/28 03:11:18 tg Exp $");
#define XCOL col
#else
#define XCOL (scr + x - ox)
#endif

 LGEN_LABEL(loop):
	/* Display next character */
	if (!amnt)
		goto LGEN_LABEL(noamnt);
	do {
		if (ungetit == -1)
			bc = *bp++;
		else {
			bc = ungetit;
			ungetit = -1;
		}
		if (st != -1)
			atr = syn[idx++];
		if (utf8_sm.state && !(bc & 0x80))
			goto LGEN_LABEL(ilseq);
		if (p->b->o.crlf && bc == '\r') {
			++byte;
			if (!--amnt) {
 LGEN_LABEL(pl):
				if (bp == p->ptr + SEGSIZ) {
					if (pnext(p)) {
						bp = p->ptr;
						amnt = p->hdr->hole;
					} else
						goto LGEN_LABEL(nl);
				} else {
					bp = p->ptr + p->hdr->ehole;
					amnt = SEGSIZ - p->hdr->ehole;
					if (!amnt)
						goto LGEN_LABEL(pl);
				}
			}
			if (*bp == '\n') {
				++bp;
				++byte;
				++amnt;
				goto eobl;
			}
 LGEN_LABEL(nl):
			--byte;
			++amnt;
		}
		if (square)
			if (bc == '\t') {
				long tcol = XCOL + p->b->o.tab -
				    (XCOL % p->b->o.tab);

				if (tcol > from && tcol <= to)
					c1 = INVERSE;
				else
					c1 = 0;
			} else if (XCOL >= from && XCOL < to)
				c1 = INVERSE;
			else
				c1 = 0;
		else if (byte >= from && byte < to)
			c1 = INVERSE;
		else
			c1 = 0;
		++byte;
		if (bc == '\t') {
			ta = p->b->o.tab - (XCOL % p->b->o.tab);
#ifdef LGEN_Q
			if (ta + col > scr) {
				ta -= scr - col;
				goto dota_tab;
			}
			if ((col += ta) == scr) {
				--amnt;
				goto v_loop;
			}
#else
 dota_tab:
			tach1 = tach = ' ';
			if (bw->o.vispace)
				tach = 0x2192;
			goto dota_gen;
 dota_loop:
			tach = tach1;
 dota_gen:
			outatr(utf8_map, t, screen + x, attr + x,
			    x, y, tach, c1 | atr);
			if (have)
				goto bye;
			if (++x == w)
				goto eosl;
			if (--ta)
				goto dota_loop;
#endif
		} else if (bc == '\n') {
			goto eobl;
		} else {
			int wid = -1;

			/*
			 * should be p->b->o.charmap->type logically
			 * but they are identical here, and it helps
			 * static analysers like Coverity
			 */
			if (bw->b->o.charmap->type) {
				/* UTF-8 */
				c = utf8_decode(&utf8_sm, bc);

				if (c >= 0) {
					/* Normal decoded character */
					wid = joe_wcwidth(c);
				} else if (c == -1) {
					/* Character taken */
					wid = -1;
				} else if (c == -2) {
					/* Incomplete sequence */
					--byte;
 LGEN_LABEL(ilseq):
					++amnt;
					ungetit = bc;
					c = 0x1000FFFE;
					wid = utf8_sm.ptr;
					utf8_init(&utf8_sm);
				} else if (c == -3) {
					/* Invalid byte */
					c = 0x1000FFFE;
					wid = 1;
				}
			} else {
				/* 8-bit charmap */
				c = bc;
				wid = 1;
			}

			if (wid >= 0) {
#ifdef LGEN_Q
				col += wid;
				if (col == scr) {
					--amnt;
					goto v_loop;
				} else if (col > scr) {
					ta = col - scr;
					tach1 = tach = '<';
					goto dota_gen;
				}
#else
				if (x + wid > w) {
					/* Character hits rightmost column */
					while (x < w) {
						/* Display offscreen marker */
						outatr(bw->b->o.charmap,
						    t, screen + x, attr + x,
						    x, y, '>', c1 | atr);
						x++;
					}
				} else if (c == 0x1000FFFE) while (wid--) {
					outatr(bw->b->o.charmap, t,
					    screen + x, attr + x,
					    x, y, 0xFFFD,
					    (c1 | atr | UNDERLINE) ^ INVERSE);
					x++;
				} else if (c == 0x20 && bw->o.vispace) {
					outatr(utf8_map, t,
					    screen + x, attr + x,
					    x, y, 0xB7, c1 | atr);
					x++;
				} else {
					outatr(bw->b->o.charmap, t,
					    screen + x, attr + x,
					    x, y, c, c1 | atr);
					x += wid;
				}
#endif
			} else
				/* Get highlighting character again */
				--idx;
#ifndef LGEN_Q
			if (have)
				goto bye;
			if (x >= w)
				goto eosl;
#endif
		}
	} while (--amnt);
 LGEN_LABEL(noamnt):
	if (bp == p->ptr + SEGSIZ) {
		if (pnext(p)) {
			bp = p->ptr;
			amnt = p->hdr->hole;
			goto LGEN_LABEL(loop);
		}
	} else {
		bp = p->ptr + p->hdr->ehole;
		amnt = SEGSIZ - p->hdr->ehole;
		goto LGEN_LABEL(loop);
	}
	if (utf8_sm.state)
		goto LGEN_LABEL(ilseq);
	goto eof;

#undef XCOL
