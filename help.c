/*
 *	Help system
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *		(C) 2001 Marek 'Marx' Grac
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#include "config.h"
#include "types.h"

__RCSID("$MirOS: contrib/code/jupp/help.c,v 1.18 2018/01/08 00:40:44 tg Exp $");

#include <stdlib.h>
#include <string.h>

#ifdef HAVE_BSD_STRING_H
#include <bsd/string.h>
#endif

#include "blocks.h"
#include "builtin.h"
#include "help.h"
#include "scrn.h"
#include "utils.h"
#include "vs.h"
#include "utf8.h"
#include "w.h"

struct help *help_actual = NULL;			/* actual help screen */

/*
 * Process help file
 */
void
help_init(const unsigned char *filename)
{
	JFILE *fd;					/* help file */
	unsigned char buf[1024];			/* input buffer */
	struct help *tmp;
	char *cp;

	/* open the help file */
	if (!(fd = jfopen(filename, "r")))
		return;

	fprintf(stderr, "Processing '%s'...", filename);
	fflush(stderr);

	while (jfgets(buf, sizeof(buf), fd)) {
		if (buf[0] == '{'/*}*/) {
			/* start of help screen */
			tmp = calloc(1, sizeof(struct help));
			/* drop newline */
			buf[strlen(buf) - 1] = '\0';
			tmp->name = (unsigned char *)strdup((char *)buf + 1);
			/* read text */
			while (jfgets(buf, sizeof(buf), fd) && buf[0] != /*{*/'}') {
				tmp->text = vsncpy(sv(tmp->text), sz(buf));
				++tmp->lines;
			}
			/* end of help screen */
			if (buf[0] != /*{*/'}') {
				fprintf(stderr, /*{*/ "\nHelp file '%s' is not properly ended with } on new line.\nDo you want to accept incomplete help screen (y/n)?", filename);
				fflush(stderr);
				if (!fgets((char *)buf, sizeof(buf), stdin) ||
				    (buf[0] | 0x20) != 'y') {
					vsrm(tmp->text);
					free(tmp->name);
					free(tmp);
					goto out;
				}
			}
			/* intern string to save memory */
			if ((cp = strdup((char *)tmp->text))) {
				vsrm(tmp->text);
				tmp->text = (unsigned char *)cp;
			}
			/* set new help screen as actual one */
			tmp->prev = help_actual;
			if (help_actual)
				help_actual->next = tmp;
			help_actual = tmp;
		}
	}
	fprintf(stderr, "done\n");
 out:
	/* close help file */
	jfclose(fd);

	/* move to first help screen */
	while (help_actual && help_actual->prev)
		help_actual = help_actual->prev;
}

/*
 * Find context help - find help entry with the same name
 */

struct help *
find_context_help(const unsigned char *name)
{
	struct help *tmp = help_actual;

	while (tmp->prev != NULL)	/* find the first help entry */
		tmp = tmp->prev;

	while (tmp != NULL && strcmp(tmp->name, name) != 0)
		tmp = tmp->next;

	return tmp;
}

/*
 * Display help text
 */
void
help_display(SCREEN *t)
{
	const unsigned char *str;
	int y, x, z;
	int atr = 0;

	if (help_actual) {
		str = help_actual->text;
	} else {
		str = UC "";
	}

	for (y = skiptop; y != t->wind; ++y) {
		if (t->t->updtab[y]) {
			const unsigned char *start = str;
			int width=0;
			int nspans=0;
			int spanwidth;
			int spancount=0;
			int spanextra;
			/* First pass: count no. springs \| and determine minimum width */
			while(*str && *str!='\n')
				if (*str++ == '\\')
					switch(*str) {
						case 'i':
						case 'I':
						case 'u':
						case 'U':
						case 'd':
						case 'D':
						case 'b':
						case 'B':
						case 'f':
						case 'F':
							++str;
							break;
						case '|':
							++str;
							++nspans;
							break;
						case 0:
							break;
						default:
							++str;
							++width;
					}
				else
					++width;
			str = start;
			/* Now calculate span width */
			if (width >= t->w - 1 || nspans==0) {
				spanwidth = 0;
				spanextra = nspans;
			} else {
				spanwidth = ((t->w - 1) - width)/nspans;
				spanextra = nspans - ((t->w - 1) - width - nspans*spanwidth);
			}
			/* Second pass: display text */
			for (x = 0; x != t->w - 1; ++x) {
				if (*str == '\n' || !*str) {
					if (eraeol(t->t, x, y)) {
						return;
					} else {
						break;
					}
				} else {
					if (*str == '\\') {
						switch (*++str) {
						case '|':
							++str;
							for (z=0;z!=spanwidth;++z)
								outatr(locale_map,t->t,t->t->scrn+x+y*t->w+z,t->t->attr+x+y*t->w+z,x+z,y,' ',atr);
							if (spancount++ >= spanextra) {
								outatr(locale_map,t->t,t->t->scrn+x+y*t->w+z,t->t->attr+x+y*t->w+z,x+z,y,' ',atr);
								++z;
							}
							x += z-1;
							continue;
						case 'i':
						case 'I':
							atr ^= INVERSE;
							++str;
							--x;
							continue;
						case 'u':
						case 'U':
							atr ^= UNDERLINE;
							++str;
							--x;
							continue;
						case 'd':
						case 'D':
							atr ^= DIM;
							++str;
							--x;
							continue;
						case 'b':
						case 'B':
							atr ^= BOLD;
							++str;
							--x;
							continue;
						case 'f':
						case 'F':
							atr ^= BLINK;
							++str;
							--x;
							continue;
						case 0:
							--x;
							continue;
						}
					}
					outatr_help(t->t,
					    t->t->scrn + x + y * t->w,
					    t->t->attr + x + y * t->w,
					    x, y, *str++, atr);
				}
			}
			atr = 0;
			t->t->updtab[y] = 0;
		}

		while (*str && *str != '\n')
			++str;
		if (*str == '\n')
			++str;
	}
}

/*
 * Show help screen
 */
int
help_on(SCREEN *t)
{
	if (help_actual) {
		t->wind = help_actual->lines + skiptop;
		if ((t->h - t->wind) < FITHEIGHT) {
			t->wind = t->h - FITHEIGHT;
		}
		if (t->wind < 0) {
			t->wind = skiptop;
			return -1;
		}
		wfit(t);
		msetI(t->t->updtab + skiptop, 1, t->wind);
		return 0;
	} else {
		return -1;
	}
}

/*
 * Hide help screen
 */
void
help_off(SCREEN *t)
{
	t->wind = skiptop;
	wfit(t);
}

/*
 * Show/hide current help screen
 */
int
u_help(BASE *base)
{
	W *w = base->parent;
	struct help *new_help;

	if (w->huh && (new_help = find_context_help(w->huh)) != NULL) {
		if (help_actual != new_help) {
			if (w->t->wind != skiptop)
				help_off(w->t);
			help_actual = new_help;		/* prepare context help */
		}
	}
	if (w->t->wind == skiptop) {
		return help_on(w->t);			/* help screen is hidden, so show the actual one */
	} else {
		help_off(w->t);				/* hide actual help screen */
		return 0;
	}
}

/*
 * Show next help screen (if it is possible)
 */
int
u_help_next(BASE *base)
{
	W *w = base->parent;

	if (help_actual && help_actual->next) {		/* is there any next help screen? */
		if (w->t->wind != skiptop) {
			help_off(w->t);			/* if help screen was visible, then hide it */
		}
		help_actual = help_actual->next;	/* change to next help screen */
		return help_on(w->t);			/* show actual help screen */
	} else {
		return -1;
	}
}

/*
 * Show previous help screen (if it is possible)
 */
int
u_help_prev(BASE *base)
{
	W *w = base->parent;

	if (help_actual && help_actual->prev) {		/* is there any previous help screen? */
		if (w->t->wind != skiptop)
			help_off(w->t);			/* if help screen was visible, then hide it */
		help_actual = help_actual->prev;	/* change to previous help screen */
		return help_on(w->t);			/* show actual help screen */
	} else {
		return -1;
	}
}
