/*
 *	*rc file parser
 *	Copyright
 *		(C) 1992 Joseph H. Allen;
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#define EXTERN_RC_C
#include "config.h"
#include "types.h"

__RCSID("$MirOS: contrib/code/jupp/rc.c,v 1.47 2018/02/14 17:45:30 tg Exp $");

#include <string.h>
#include <stdlib.h>

#ifdef HAVE_BSD_STRING_H
#include <bsd/string.h>
#endif

#include "builtin.h"
#include "cmd.h"
#include "kbd.h"
#include "macro.h"
#include "menu.h"
#include "path.h"
#include "pw.h"
#include "rc.h"
#include "regex.h"
#include "tw.h"
#include "uedit.h"
#include "umath.h"
#include "utils.h"
#include "vs.h"
#include "b.h"
#include "syntax.h"
#include "va.h"
#include "charmap.h"
#include "w.h"

#define OPT_BUF_SIZE	60

/* List of named contexts */
static struct context {
	struct context *next;
	unsigned char *name;
	KMAP *kmap;
} *contexts = NULL;

/*
 * Find a context of a given name; if not found,
 * one with an empty kmap is created.
 */
KMAP *
kmap_getcontext(const unsigned char *name, int docreate)
{
	struct context *c;

	for (c = contexts; c; c = c->next)
		if (!strcmp(c->name, name))
			return c->kmap;

	if (!docreate)
		return (NULL);

	c = malloc(sizeof(struct context));

	c->next = contexts;
	c->name = (unsigned char *)strdup((char *)name);
	contexts = c;
	return c->kmap = mkkmap();
}

OPTIONS *options = NULL;

/* Global variable options */
extern int mid, dspasis, dspctrl, help, square, csmode, nobackups, lightoff, exask, skiptop;
extern int noxon, lines, columns, Baud, dopadding, orphan, marking, keepup, nonotice;
extern int notite, pastetite, usetabs, assume_color, guesscrlf, guessindent, menu_explorer, icase, wrap, autoswap;
extern unsigned char *backpath;

/* Default options for prompt windows */
OPTIONS pdefault = {
	NULL,		/* *next */
	NULL,		/* *name_regex */
	NULL,		/* *contents_regex */
	0,		/* overtype */
	0,		/* lmargin */
	76,		/* rmargin */
	0,		/* autoindent */
	0,		/* wordwrap */
	8,		/* tab */
	' ',		/* indent char */
	1,		/* indent step */
	NULL,		/* *context */
	NULL,		/* *lmsg */
	NULL,		/* *rmsg */
	NULL,		/* *hmsg */
	0,		/* line numbers */
	0,		/* read only */
	0,		/* french spacing */
	0,		/* spaces */
	0,		/* crlf */
	0,		/* Highlight */
	NULL,		/* Syntax name */
	NULL,		/* Syntax */
	NULL,		/* Name of character set */
	NULL,		/* Character set */
	0,		/* Smart home key */
	0,		/* Goto indent first */
	0,		/* Smart backspace key */
	0,		/* Purify indentation */
	0,		/* Picture mode */
	NULL,		/* macro to execute for new files */
	NULL,		/* macro to execute for existing files */
	NULL,		/* macro to execute before saving new files */
	NULL,		/* macro to execute before saving existing files */
	0,		/* visible spaces */
	0		/* hex */
};

/* Default options for file windows */
char main_context[] = "main";
OPTIONS fdefault = {
	NULL,		/* *next */
	NULL,		/* *name_regex */
	NULL,		/* *contents_regex */
	0,		/* overtype */
	0,		/* lmargin */
	76,		/* rmargin */
	0,		/* autoindent */
	0,		/* wordwrap */
	8,		/* tab */
	' ',		/* indent char */
	1,		/* indent step */
	US main_context,		/* *context */
	UC "\\i%n %m %M",		/* *lmsg */
	UC " %S Ctrl-K H for help",	/* *rmsg */
	NULL,		/* *hmsg */
	0,		/* line numbers */
	0,		/* read only */
	0,		/* french spacing */
	0,		/* spaces */
	0,		/* crlf */
	0,		/* Highlight */
	NULL,		/* Syntax name */
	NULL,		/* Syntax */
	NULL,		/* Name of character set */
	NULL,		/* Character set */
	0,		/* Smart home key */
	0,		/* Goto indent first */
	0,		/* Smart backspace key */
	0,		/* Purity indentation */
	0,		/* Picture mode */
	NULL, NULL, NULL, NULL,	/* macros (see above) */
	0,		/* visible spaces */
	0		/* hex */
};

/* Update options */
void
lazy_opts(OPTIONS *o)
{
	o->syntax = load_dfa(o->syntax_name);
	o->charmap = find_charmap(o->map_name);
	if (!o->charmap)
		o->charmap = fdefault.charmap;
	/* Hex not allowed with UTF-8 */
	if (o->hex && o->charmap->type) {
		o->charmap = find_charmap(UC "c");
	}
}

/* Set local options depending on file name and contents */
void
setopt(B *b, const unsigned char *parsed_name)
{
	OPTIONS *o;
	int x;
	unsigned char *pieces[26];
	P *p;

	for (x = 0; x != 26; ++x)
		pieces[x] = NULL;

	for (o = options; o; o = o->next)
		if (rmatch(o->name_regex, parsed_name)) {
			if (!o->contents_regex)
				goto done;
			p = pdup(b->bof);
			x = pmatch(pieces, o->contents_regex,
			    strlen((char *)o->contents_regex), p, 0, 0);
			prm(p);
			if (x)
				goto done;
		}

	b->o = fdefault;
	if (0) {
 done:
		b->o = *o;
	}
	lazy_opts(&b->o);

	for (x = 0; x != 26; ++x)
		vsrm(pieces[x]);
}

/* Table of options and how to set them */

/*
 * local means it's in an OPTION structure,
 * global means it's in a global variable
 */
#define F(x) NULL, &fdefault.x
#define G(type,name,setiaddr,yes,no,menu,low,high) \
	X(type,name,setiaddr,yes,no,menu,low,high)
#define X(type,name,seti,addr,yes,no,menu,low,high) \
	{ UC name, { seti }, US addr, UC yes, UC no, UC menu, 0, type, low, high }
#define L(x) &x, NULL
static struct glopts {
	const unsigned char *name;	/* Option name */
	union {
		int *i;
		unsigned char **us;
	} set;				/* Address of global option */
	unsigned char *addr;		/* Local options structure member address */
	const unsigned char *yes;	/* Message if option was turned on, or prompt string */
	const unsigned char *no;	/* Message if option was turned off */
	const unsigned char *menu;	/* Menu string */
	size_t ofst;			/* Local options structure member offset */
	int type;		/* 0 for global option flag
				   1 for global option numeric
				   2 for global option string
				   4 for local option flag
				   5 for local option numeric
				   6 for local option string
				   7 for local option numeric+1, with range checking
				   8 for ...?
				   9 for syntax
				  13 for encoding
				 */
	int low;		/* Low limit for numeric options */
	int high;		/* High limit for numeric options */
} glopts[] = {
G( 0, "noxon",		L(noxon),	  "XON/XOFF processing disabled", "XON/XOFF processing enabled", "  XON/XOFF usable ", 0, 0),
G( 0, "keepup",		L(keepup),	  "Status line updated constantly", "Status line updated once/sec", "  Fast status line ", 0, 0),
G( 1, "baud",		L(Baud),	  "Terminal baud rate (%d): ", NULL, "  Baud rate ", 0, 38400),
G( 4, "overwrite",	F(overtype),	  "Overtype mode", "Insert mode", "T Overtype ", 0, 0),
G( 4, "autoindent",	F(autoindent),	  "Autoindent enabled", "Autoindent disabled", "I Autoindent ", 0, 0),
G( 4, "wordwrap",	F(wordwrap),	  "Wordwrap enabled", "Wordwrap disabled", "Word wrap ", 0, 0),
G( 5, "tab",		F(tab),		  "Tab width (%d): ", NULL, "D Tab width ", 1, 64),
G( 7, "lmargin",	F(lmargin),	  "Left margin (%d): ", NULL, "Left margin ", 0, 63),
G( 7, "rmargin",	F(rmargin),	  "Right margin (%d): ", NULL, "Right margin ", 7, 255),
G( 0, "square",		L(square),	  "Rectangle mode", "Text-stream mode", "X Rectangle mode ", 0, 0),
G( 0, "icase",		L(icase),	  "Ignore case by default", "Case sensitive by default", "  Case insensitive ", 0, 0),
G( 0, "wrap",		L(wrap),	  "Search wraps", "Search doesn't wrap", "  Search wraps ", 0, 0),
G( 0, "menu_explorer",	L(menu_explorer), "Menu explorer mode", "  Simple completion", "  Menu explorer ", 0, 0),
G( 0, "autoswap",	L(autoswap),	  "Autoswap ^KB and ^KK", "  Autoswap off ", "  Autoswap mode ", 0, 0),
G( 5, "indentc",	F(indentc),	  "Indent char %d (SPACE=32, TAB=9, ^C to abort): ", NULL, "  Indent char ", 0, 255),
G( 5, "istep",		F(istep),	  "Indent step %d (^C to abort): ", NULL, "  Indent step ", 1, 64),
G( 4, "french",		F(french),	  "One space after periods for paragraph reformat", "Two spaces after periods for paragraph reformat", "  French spacing ", 0, 0),
G( 4, "highlight",	F(highlight),	  "Highlighting enabled", "Highlighting disabled", "Highlighting ", 0, 0),
G( 4, "spaces",		F(spaces),	  "Inserting spaces when tab key is hit", "Inserting tabs when tab key is hit", "  Disable tabs ", 0, 0),
G( 0, "mid",		L(mid),		  "Cursor will be recentered on scrolls", "Cursor will not be recentered on scroll", "Center on scroll ", 0, 0),
G( 0, "guess_crlf",	L(guesscrlf),	  "Automatically detect MS-DOS files", "Do not automatically detect MS-DOS files", "  Auto detect CR-LF ", 0, 0),
G( 0, "guess_indent",	L(guessindent),	  "Automatically detect indentation", "Do not automatically detect indentation", "  Guess indent ", 0, 0),
G( 4, "crlf",		F(crlf),	  "CR-LF is line terminator", "LF is line terminator", "Z CR-LF (MS-DOS) ", 0, 0),
G( 4, "linums",		F(linums),	  "Line numbers enabled", "Line numbers disabled", "N Line numbers ", 0, 0),
G( 0, "marking",	L(marking),	  "Anchored block marking on", "Anchored block marking off", "Marking ", 0, 0),
G( 0, "asis",		L(dspasis),	  "Characters above 127 shown as-is", "Characters above 127 shown in inverse", "  Meta chars as-is ", 0, 0),
G( 0, "force",		L(force),	  "Last line forced to have NL when file saved", "Last line not forced to have NL", "Force last NL ", 0, 0),
G( 0, "nobackups",	L(nobackups),	  "Backup files will not be made", "Backup files will be made", "  Disable backups ", 0, 0),
G( 0, "lightoff",	L(lightoff),	  "Highlighting turned off after block operations", "Highlighting not turned off after block operations", "Auto unmark ", 0, 0),
G( 0, "exask",		L(exask),	  "Prompt for filename in save & exit command", "Don't prompt for filename in save & exit command", "  Exit ask ", 0, 0),
G( 0, "beep",		L(dobeep),	  "Warning bell enabled", "Warning bell disabled", "Beeps ", 0, 0),
G( 0, "nosta",		L(staen),	  "Top-most status line disabled", "Top-most status line enabled", "  Disable status ", 0, 0),
G( 1, "pg",		L(pgamnt),	  "Lines to keep for PgUp/PgDn or -1 for 1/2 window (%d): ", NULL, "  # PgUp/PgDn lines ", -1, 64),
G( 0, "csmode",		L(csmode),	  "Start search after a search repeats previous search", "Start search always starts a new search", "Continued search ", 0, 0),
G( 4, "rdonly",		F(readonly),	  "Read only", "Full editing", "O Read only ", 0, 0),
G( 4, "smarthome",	F(smarthome),	  "Smart home key enabled", "Smart home key disabled", "  Smart home key ", 0, 0),
G( 4, "indentfirst",	F(indentfirst),	  "Smart home goes to indent first", "Smart home goes home first", "  To indent first ", 0, 0),
G( 4, "smartbacks",	F(smartbacks),	  "Smart backspace key enabled", "Smart backspace key disabled", "  Smart backspace ", 0, 0),
G( 4, "purify",		F(purify),	  "Indentation clean up enabled", "Indentation clean up disabled", "  Clean up indents ", 0, 0),
G( 4, "picture",	F(picture),	  "Picture drawing mode enabled", "Picture drawing mode disabled", "Picture mode ", 0, 0),
X( 2, "backpath",	NULL, NULL,	  "Backup files stored in (%s): ", NULL, "  Backup file path ", 0, 0),
G( 4, "vispace",	F(vispace),	  "Spaces visible", "Spaces invisible", "Visible spaces ", 0, 0),
G( 4, "hex",		F(hex),		  "Hex edit mode", "Text edit mode", "G Hexedit mode ", 0, 0),
X( 9, "syntax",		NULL, NULL,	  "Select syntax (%s; ^C to abort): ", NULL, "Y Syntax", 0, 0),
X(13, "encoding",	NULL, NULL,	  "Select file character set (%s; ^C to abort): ", NULL, "Encoding ", 0, 0),
G( 0, "nonotice",	L(nonotice),	  NULL, NULL, NULL, 0, 0),
G( 0, "orphan",		L(orphan),	  NULL, NULL, NULL, 0, 0),
G( 0, "help",		L(help),	  NULL, NULL, NULL, 0, 0),
G( 0, "dopadding",	L(dopadding),	  NULL, NULL, NULL, 0, 0),
G( 1, "lines",		L(lines),	  NULL, NULL, NULL, 2, 1024),
G( 1, "columns",	L(columns),	  NULL, NULL, NULL, 2, 1024),
G( 1, "skiptop",	L(skiptop),	  NULL, NULL, NULL, 0, 64),
G( 0, "notite",		L(notite),	  NULL, NULL, NULL, 0, 0),
G( 0, "pastetite",	L(pastetite),	  NULL, NULL, NULL, 0, 0),
G( 0, "usetabs",	L(usetabs),	  NULL, NULL, NULL, 0, 0),
G( 0, "assume_color",	L(assume_color),  NULL, NULL, NULL, 0, 0),
X( 0, NULL,		NULL, NULL,	  NULL, NULL, NULL, 0, 0)
};
#undef F
#undef G
#undef L
#undef X

/* Initialize .ofsts above.  Is this really necessary? */

int isiz = 0;

static void izopts(void)
{
	int x;

	for (x = 0; glopts[x].name; ++x)
		switch (glopts[x].type) {
		case 2:
			if (!strcmp((const char *)glopts[x].name, "backpath"))
				glopts[x].set.us = &backpath;
			break;
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
			glopts[x].ofst = glopts[x].addr - (unsigned char *) &fdefault;
		}
	isiz = 1;
}
#define RELOPT(lopts, opt) (*((int *)(((unsigned char *)(lopts)) + glopts[opt].ofst)))

/*-
 * Set a global or local option:
 * 's' is option name
 * 'arg' is a possible argument string (taken only if option has an arg)
 * 'options' points to options structure to modify (can be NULL).
 * 'set'==0: set only in 'options' if it's given.
 * 'set'!=0: set global variable option.
 * return value: no. of fields taken (1 or 2), or 0 if option not found.
 *
 * So this function is used both to set options, and to parse over options
 * without setting them.
 *
 * These combinations are used:
 *
 * glopt(name,arg,NULL,1): set global variable option
 * glopt(name,arg,NULL,0): parse over option
 * glopt(name,arg,options,0): set file local option
 * glopt(name,arg,&fdefault,1): set default file options
 * glopt(name,arg,options,1): set file local option
 */
int
glopt(unsigned char *s, unsigned char *arg, OPTIONS *opts, int set)
{
	int val;
	int ret = 0;
	int st = 1;	/* 1 to set option, 0 to clear it */
	int x;
	void *vp;

	/* Initialize offsets */
	if (!isiz)
		izopts();

	/* Clear instead of set? */
	if (s[0] == '-') {
		st = 0;
		++s;
	}

	for (x = 0; glopts[x].name; ++x)
		if (!strcmp(glopts[x].name, s)) {
			switch (glopts[x].type) {
			case 0:
				/* Global variable flag option */
				if (set)
					*glopts[x].set.i = st;
				break;
			case 1:
				/* Global variable integer option */
				if (set && arg) {
					val = ustolb(arg, &vp, glopts[x].low, glopts[x].high, USTOL_TRIM | USTOL_EOS);
					if (vp)
						*glopts[x].set.i = val;
				}
				break;
			case 2:
				/* Global variable string option */
				if (set)
					*glopts[x].set.us = arg ? (unsigned char *)strdup((char *)arg) : NULL;
				break;
			case 4:
				/* Local option flag */
				if (opts)
					RELOPT(opts, x) = st;
				break;
			case 5:
				/* Local option integer */
				if (arg && opts) {
					val = ustolb(arg, &vp, glopts[x].low, glopts[x].high, USTOL_TRIM | USTOL_EOS);
					if (vp)
						RELOPT(opts, x) = val;
				}
				break;
			case 7:
				/* Local option numeric + 1, with range checking */
				if (arg) {
					val = ustolb(arg, &vp, glopts[x].low, glopts[x].high, USTOL_TRIM | USTOL_EOS);
					if (vp && opts)
						RELOPT(opts, x) = val - 1;
				}
				break;

			case 9:
				/* Set syntax */
				if (arg && opts)
					opts->syntax_name = (unsigned char *)strdup((char *)arg);
				break;

			case 13:
				/* Set byte mode encoding */
				if (arg && opts)
					opts->map_name = (unsigned char *)strdup((char *)arg);
				break;
			}
			/* This is a stupid hack... */
			if ((glopts[x].type & 3) == 0 || !arg)
				return 1;
			else
				return 2;
		}
	/* Why no case 6, string option? */
	/* Keymap, mold, mnew, etc. are not strings */
	/* These options do not show up in ^T */
	if (!strcmp(s, "lmsg")) {
		if (arg) {
			if (opts)
				opts->lmsg = (unsigned char *)strdup((char *)arg);
			ret = 2;
		} else
			ret = 1;
	} else if (!strcmp(s, "rmsg")) {
		if (arg) {
			if (opts)
				opts->rmsg = (unsigned char *)strdup((char *)arg);
			ret = 2;
		} else
			ret = 1;
	} else if (!strcmp(s, "hmsg")) {
		if (arg) {
			if (opts)
				opts->hmsg = strdup((char *)arg);
			ret = 2;
		} else
			ret = 1;
	} else if (!strcmp(s, "keymap")) {
		if (arg) {
			if (opts)
				opts->context = (unsigned char *)strdup((char *)arg);
			ret = 2;
		} else
			ret = 1;
	} else if (!strcmp(s, "mnew")) {
		if (arg) {
			int sta;

			if (opts)
				opts->mnew = mparse(NULL, arg, &sta);
			ret = 2;
		} else
			ret = 1;
	} else if (!strcmp(s, "mold")) {
		if (arg) {
			int sta;

			if (opts)
				opts->mold = mparse(NULL, arg, &sta);
			ret = 2;
		} else
			ret = 1;
	} else if (!strcmp(s, "msnew")) {
		if (arg) {
			int sta;

			if (opts)
				opts->msnew = mparse(NULL, arg, &sta);
			ret = 2;
		} else
			ret = 1;
	} else if (!strcmp(s, "msold")) {
		if (arg) {
			int sta;

			if (opts)
				opts->msold = mparse(NULL, arg, &sta);
			ret = 2;
		} else
			ret = 1;
	}

	return ret;
}

/* Option setting user interface (^T command) */

/* Menu cursor position: remember it for next time */
static int optx = 0;

static int
doabrt1(BW *bw, int *xx)
{
	free(xx);
	return -1;
}

static int
doopt1(BW *bw, unsigned char *s, int *xx, int *notify)
{
	int ret = 0;
	int x = *xx;
	long v;

	free(xx);
	switch (glopts[x].type) {
	case 1:
		if (!*s) {
			ret = -1;
			break;
		}
		v = calcl(bw, s);
		if (merrf) {
			msgnw(bw->parent, merrt);
			ret = -1;
		} else if (v >= glopts[x].low && v <= glopts[x].high)
			*glopts[x].set.i = v;
		else {
			msgnw(bw->parent, UC "Value out of range");
			ret = -1;
		}
		break;
	case 2:
		if (s[0])
			*glopts[x].set.us = (unsigned char *)strdup((char *)s);
		break;
	case 5:
		if (!*s) {
			ret = -1;
			break;
		}
		v = calcl(bw, s);
		if (merrf) {
			msgnw(bw->parent, merrt);
			ret = -1;
		} else if (v >= glopts[x].low && v <= glopts[x].high)
			RELOPT(&bw->o, x) = v;
		else {
			msgnw(bw->parent, UC "Value out of range");
			ret = -1;
		}
		break;
	case 7:
		if (!*s) {
			ret = -1;
			break;
		}
		v = calcl(bw, s) - 1L;
		if (merrf) {
			msgnw(bw->parent, merrt);
			ret = -1;
		} else if (v >= glopts[x].low && v <= glopts[x].high)
			RELOPT(&bw->o, x) = v;
		else {
			msgnw(bw->parent, UC "Value out of range");
			ret = -1;
		}
		break;
	}
	vsrm(s);
	bw->b->o = bw->o;
	wfit(bw->parent->t);
	updall();
	if (notify)
		*notify = 1;
	return ret;
}

static int
dosyntax(BW *bw, unsigned char *s, int *xx, int *notify)
{
	int ret = 0;
	struct high_syntax *syn;

	if (*s) {
		if ((syn = load_dfa(s)))
			bw->o.syntax = syn;
		else
			msgnw(bw->parent, UC "Syntax definition file not found");
	} else
		bw->o.syntax = NULL;

	vsrm(s);
	bw->b->o = bw->o;
	updall();
	if (notify)
		*notify = 1;
	return ret;
}

/* Array of available syntacēs */
unsigned char **syntaxes = NULL;

static int
syntaxcmplt(BW *bw)
{
	if (!syntaxes) {
		unsigned char *oldpwd = pwd();
		unsigned char **t;
		unsigned char *p;
		int x, y;

		if (chJpwd(UC "syntax"))
			return -1;
		t = rexpnd(UC "*.jsf");
		if (!t) {
			chpwd(oldpwd);
			return -1;
		}
		if (!aLEN(t)) {
			varm(t);
			chpwd(oldpwd);
			return -1;
		}

		for (x = 0; x != aLEN(t); ++x) {
			unsigned char *r = vsncpy(NULL,0,t[x],(unsigned char *)strrchr((char *)(t[x]),'.')-t[x]);
			syntaxes = vaadd(syntaxes,r);
		}
		varm(t);

		p = (unsigned char *)getenv("HOME");
		if (p) {
			unsigned char buf[1024];
			joe_snprintf_1((char *)buf,sizeof(buf),"%s/.jupp/syntax",p);
			if (!chpwd(buf) && (t = rexpnd(UC "*.jsf"))) {
				for (x = 0; x != aLEN(t); ++x)
					*strrchr((char *)t[x],'.') = 0;
				for (x = 0; x != aLEN(t); ++x) {
					for (y = 0; y != aLEN(syntaxes); ++y)
						if (!strcmp(t[x],syntaxes[y]))
							break;
					if (y == aLEN(syntaxes)) {
						unsigned char *r = vsncpy(NULL,0,sv(t[x]));
						syntaxes = vaadd(syntaxes,r);
					}
				}
				varm(t);
			}
		}

		vasort(av(syntaxes));
		chpwd(oldpwd);
	}
	return simple_cmplt(bw,syntaxes);
}

static int
check_for_hex(BW *bw)
{
	W *w;
	if (bw->o.hex)
		return 1;
	for (w = bw->parent->link.next; w != bw->parent; w = w->link.next)
		if ((w->watom == &watomtw || w->watom == &watompw) &&
		    w->object.bw->b == bw->b && w->object.bw->o.hex)
			return 1;
	return 0;
}

static int
doencoding(BW *bw, unsigned char *s, int *xx, int *notify)
{
	int ret = 0;
	struct charmap *map;

	if (*s)
		map = find_charmap(s);
	else
		map = fdefault.charmap;

	if (map && map->type && check_for_hex(bw)) {
		vsrm(s);
		msgnw(bw->parent, UC "UTF-8 encoding not allowed with hex-edit windows");
		if (notify)
			*notify = 1;
		return -1;
	}

	if (map) {
		bw->o.charmap = map;
		joe_snprintf_1((char *)msgbuf, JOE_MSGBUFSIZE, "%s encoding assumed for this file", map->name);
		msgnw(bw->parent, msgbuf);
	} else
		msgnw(bw->parent, UC "Character set not found");

	vsrm(s);
	bw->b->o = bw->o;
	updall();
	if (notify)
		*notify = 1;
	return ret;
}

/* Array of available encodinges */
unsigned char **encodings = NULL;

static int
encodingcmplt(BW *bw)
{
	if (!encodings) {
		encodings = get_encodings();
		vasort(av(encodings));
	}
	return simple_cmplt(bw,encodings);
}

static int
doopt(MENU *m, int x, void *object, int flg)
{
	BW *bw = m->parent->win->object.bw;
	int *xx;
	unsigned char buf[OPT_BUF_SIZE];
	int *notify = m->parent->notify;

	switch (glopts[x].type) {
	case 0:
		if (!flg)
			*glopts[x].set.i = !*glopts[x].set.i;
		else if (flg == 1)
			*glopts[x].set.i = 1;
		else
			*glopts[x].set.i = 0;
		wabort(m->parent);
		msgnw(bw->parent, *glopts[x].set.i ? glopts[x].yes : glopts[x].no);
		if (glopts[x].set.i == &noxon)
			tty_xonoffbaudrst();
		break;
	case 4:
		if (!flg)
			RELOPT(&bw->o, x) = !RELOPT(&bw->o, x);
		else if (flg == 1)
			RELOPT(&bw->o, x) = 1;
		else
			RELOPT(&bw->o, x) = 0;
		wabort(m->parent);
		msgnw(bw->parent, RELOPT(&bw->o, x) ? glopts[x].yes : glopts[x].no);
		/*XXX use offsetof, also in izopts or better statically */
		if (glopts[x].ofst == (unsigned char *) &fdefault.readonly - (unsigned char *) &fdefault)
			bw->b->rdonly = bw->o.readonly;
		/* Kill UTF-8 and CR-LF mode if we switch to hex display */
		if (glopts[x].ofst == (unsigned char *)&fdefault.hex - (unsigned char *)&fdefault &&
		    bw->o.hex) {
			if (bw->b->o.charmap->type) {
				doencoding(bw, vsncpy(NULL, 0, sc("c")),
				    NULL, NULL);
			}
			bw->o.crlf = 0;
		}
		break;
	case 1:
		joe_snprintf_1((char *)buf, OPT_BUF_SIZE, (char *)glopts[x].yes, *glopts[x].set.i);
		xx = malloc(sizeof(int));

		*xx = x;
		m->parent->notify = 0;
		wabort(m->parent);
		if (wmkpw(bw->parent, buf, NULL, doopt1, NULL, doabrt1, utypebw, xx, notify, locale_map)) {
			if (glopts[x].set.i == &Baud)
				tty_xonoffbaudrst();
			return 0;
		} else
			return -1;
	case 2:
		if (*glopts[x].set.us)
			joe_snprintf_1((char *)buf, OPT_BUF_SIZE, (char *)glopts[x].yes, *glopts[x].set.us);
		else
			joe_snprintf_1((char *)buf, OPT_BUF_SIZE, (char *)glopts[x].yes, "");
		xx = malloc(sizeof(int));

		*xx = x;
		m->parent->notify = 0;
		wabort(m->parent);
		if (wmkpw(bw->parent, buf, NULL, doopt1, NULL, doabrt1, utypebw, xx, notify, locale_map))
			return 0;
		else
			return -1;
	case 5:
		joe_snprintf_1((char *)buf, OPT_BUF_SIZE,
		    (const char *)glopts[x].yes, RELOPT(&bw->o, x));
		goto in;
	case 7:
		joe_snprintf_1((char *)buf, OPT_BUF_SIZE,
		    (const char *)glopts[x].yes, RELOPT(&bw->o, x) + 1);
 in:
		xx = malloc(sizeof(int));

		*xx = x;
		m->parent->notify = 0;
		wabort(m->parent);
		if (wmkpw(bw->parent, buf, NULL, doopt1, NULL, doabrt1, utypebw, xx, notify, locale_map))
			return 0;
		else
			return -1;

	case 9:
		joe_snprintf_1((char *)buf, OPT_BUF_SIZE, (char *)glopts[x].yes,
		    bw->b->o.syntax ? bw->b->o.syntax->name : UC "(unset)");
		m->parent->notify = 0;
		wabort(m->parent);
		if (wmkpw(bw->parent, buf, NULL, dosyntax, NULL, NULL, syntaxcmplt, NULL, notify, locale_map))
			return 0;
		else
			return -1;

	case 13:
		joe_snprintf_1((char *)buf, OPT_BUF_SIZE, (char *)glopts[x].yes,
		    bw->b->o.charmap ? bw->b->o.charmap->name : UC "(unset)");
		m->parent->notify = 0;
		wabort(m->parent);
		if (wmkpw(bw->parent, buf, NULL, doencoding, NULL, NULL, encodingcmplt, NULL, notify, locale_map))
			return 0;
		else
			return -1;
	}
	if (notify)
		*notify = 1;
	bw->b->o = bw->o;
	wfit(bw->parent->t);
	updall();
	return 0;
}

static int
doabrt(MENU *m, int x, unsigned char **s)
{
	optx = x;
	for (x = 0; s[x]; ++x)
		free(s[x]);
	free(s);
	return -1;
}

int
umode(BW *bw)
{
	size_t size = 0, x, len;
	unsigned char **s;

	bw->b->o.readonly = bw->o.readonly = bw->b->rdonly;
	while (glopts[size].menu)
		++size;
	s = ralloc(size + 1, sizeof(unsigned char *));
	len = 0;
	for (x = 0; x < size; ++x) {
		s[x] = malloc(OPT_BUF_SIZE);
		if (glopts[x].menu[0] == ' ' || glopts[x].menu[1] == ' ')
			strlcpy(s[x], glopts[x].menu, OPT_BUF_SIZE);
		else {
			strlcpy(s[x] + 2, glopts[x].menu, OPT_BUF_SIZE);
			s[x][0] = s[x][2];
			s[x][1] = ' ';
		}
		if (strlen(s[x]) > len)
			len = strlen(s[x]);
	}
	for (x = 0; x < size; ++x) {
		size_t n = strlen(s[x]);

		while (len - n)
			s[x][n++] = ' ';
		switch (glopts[x].type) {
		case 0:
			joe_snprintf_1(s[x] + n, OPT_BUF_SIZE - n,
			    "%s", *glopts[x].set.i ? "ON" : "OFF");
			break;
		case 1:
			joe_snprintf_1(s[x] + n, OPT_BUF_SIZE - n,
			    "%d", *glopts[x].set.i);
			break;
		case 2:
			strlcpy(s[x] + n, "...", OPT_BUF_SIZE - n);
			break;
		case 4:
			joe_snprintf_1(s[x] + n, OPT_BUF_SIZE - n,
			    "%s", RELOPT(&bw->o, x) ? "ON" : "OFF");
			break;
		case 5:
			joe_snprintf_1(s[x] + n, OPT_BUF_SIZE - n,
			    "%d", RELOPT(&bw->o, x));
			break;
		case 7:
			joe_snprintf_1(s[x] + n, OPT_BUF_SIZE - n,
			    "%d", RELOPT(&bw->o, x) + 1);
			break;
		case 9:
			/* XXX aligns differently so it doesn't get too large */
			joe_snprintf_2(s[x] + 12, OPT_BUF_SIZE - 12, "%*s", (int)n - 9,
			    bw->b->o.syntax ? bw->b->o.syntax->name : UC "(unset)");
			break;
		case 13:
			/* XXX aligns differently so it doesn't get too large */
			joe_snprintf_2(s[x] + 12, OPT_BUF_SIZE - 12, "%*s", (int)n - 9,
			    bw->b->o.charmap ? bw->b->o.charmap->name : UC "(unset)");
			break;
		default:
			s[x][n] = '\0';
		}
	}
	s[x] = NULL;
	if (mkmenu(bw->parent, s, doopt, doabrt, NULL, optx, s, NULL))
		return 0;
	else
		return -1;
}

/*-
 * Process rc file
 * Returns 0 if the rc file was succefully processed
 *        -1 if the rc file couldn't be opened
 *         1 if there was a syntax error in the file
 */
int
procrc(CAP *cap, const unsigned char *name)
{
	OPTIONS *o = &fdefault;	/* Current options */
	KMAP *context = NULL;	/* Current context */
	unsigned char buf[1024];/* Input buffer */
	JFILE *fd;		/* rc file */
	int line = 0;		/* Line number */
	int err = 0;		/* Set to 1 if there was a syntax error */
	int x, c, y, sta;
	unsigned char *opt, *arg;
	MACRO *m;

	if (!(fd = jfopen(name, "r")))
		/* return if we couldn't open the rc file */
		return (-1);

	fprintf(stderr, "Processing '%s'...", name);
	fflush(stderr);

	while (jfgets(buf, sizeof(buf), fd)) {
		line++;
		switch (buf[0]) {
		case ' ':
		case '\t':
		case '\n':
		case '\f':
		case 0:
			/* skip comment lines */
			break;
		case '*':
			/* select file types for file type-dependent options */
			o = malloc(sizeof(OPTIONS));
			*o = fdefault;
			x = 0;
			while (buf[x] && buf[x] != '\n' && buf[x] != ' ' && buf[x] != '\t')
				++x;
			buf[x] = 0;
			o->next = options;
			options = o;
			o->name_regex = (unsigned char *)strdup((char *)buf);
			break;
		case '+':
			/* Set file contents match regex */
			x = 0;
			while (buf[x] && buf[x] != '\n' && buf[x] != '\r')
				++x;
			buf[x] = 0;
			if (o)
				o->contents_regex = (unsigned char *)strdup((char *)(buf+1));
			break;
		case '-':
			/* Set an option */
			opt = buf + 1;
			arg = NULL;
			x = 0;
			while (buf[x] && buf[x] != '\n' && buf[x] != ' ' && buf[x] != '\t')
				++x;
			if (buf[x] && buf[x] != '\n') {
				buf[x] = 0;
				arg = buf + ++x;
				while (buf[x] && buf[x] != '\n')
					++x;
			}
			buf[x] = 0;
			if (!glopt(opt, arg, o, 2)) {
				err = 1;
				fprintf(stderr, "\n%s:%d: Unknown option '%s'", name, line, opt);
			}
			break;
		case '{':
			/* Ignore help text */
			while (jfgets(buf, sizeof(buf), fd) && buf[0] != /*{*/'}')
				/* do nothing */;
			if (buf[0] != '}') {
				err = 1;
				fprintf(stderr, "\n%s:%d: End of rc file occurred before end of help text\n", name, line);
				break;
			}
			break;
		case ':':
			/* Select context */
			x = 1;
			while (!joe_isspace_eof(locale_map,buf[x]))
				++x;
			c = buf[x];
			buf[x] = 0;
			if (x != 1)
				if (!strcmp(buf + 1, "def")) {
					for (buf[x] = c; joe_isblank(locale_map,buf[x]); ++x) ;
					for (y = x; !joe_isspace_eof(locale_map,buf[y]); ++y) ;
					c = buf[y];
					buf[y] = 0;
					if (y == x) {
						err = 1;
						fprintf(stderr, "\n%s:%d: command name missing from :def", name, line);
					} else if (joe_isblank(locale_map, c) &&
					    (m = mparse(NULL, buf + y + 1, &sta)))
						addcmd(buf + x, m);
					else {
						err = 1;
						fprintf(stderr, "\n%s:%d: macro missing from :def", name, line);
					}
				} else if (!strcmp(buf + 1, "inherit"))
					if (context) {
						for (buf[x] = c; joe_isblank(locale_map,buf[x]); ++x) ;
						for (c = x; !joe_isspace_eof(locale_map,buf[c]); ++c) ;
						buf[c] = 0;
						if (c != x)
							kcpy(context, kmap_getcontext(buf + x, 1));
						else {
							err = 1;
							fprintf(stderr, "\n%s:%d: context name missing from :inherit", name, line);
						}
					} else {
						err = 1;
						fprintf(stderr, "\n%s:%d: No context selected for :inherit", name, line);
				} else if (!strcmp(buf + 1, "include")) {
					for (buf[x] = c; joe_isblank(locale_map,buf[x]); ++x) ;
					for (c = x; !joe_isspace_eof(locale_map,buf[c]); ++c) ;
					buf[c] = 0;
					if (c != x) {
						switch (procrc(cap, buf + x)) {
						case 1:
							err = 1;
							break;
						case -1:
							fprintf(stderr, "\n%s:%d: Couldn't open %s", name, line, buf + x);
							err = 1;
							break;
						}
						context = 0;
						o = &fdefault;
					} else {
						err = 1;
						fprintf(stderr, "\n%s:%d: :include missing file name", name, line);
					}
				} else if (!strcmp(buf + 1, "delete"))
					if (context) {
						for (buf[x] = c; joe_isblank(locale_map,buf[x]); ++x) ;
						for (y = x; buf[y] != 0 && buf[y] != '\t' && buf[y] != '\n' && (buf[y] != ' ' || buf[y + 1]
														!= ' '); ++y) ;
						buf[y] = 0;
						kdel(context, buf + x);
					} else {
						err = 1;
						fprintf(stderr, "\n%s:%d: No context selected for :delete", name, line);
				} else
					context = kmap_getcontext(buf + 1, 1);
			else {
				err = 1;
				fprintf(stderr, "\n%s:%d: Invalid context name", name, line);
			}
			break;
		default:
			/* Get key-sequence to macro binding */
			if (!context) {
				err = 1;
				fprintf(stderr, "\n%s:%d: No context selected for macro to key-sequence binding", name, line);
				break;
			}

			m = NULL;
 macroloop:
			m = mparse(m, buf, &x);
			if (x == -1) {
				err = 1;
				fprintf(stderr, "\n%s:%d: Unknown command in macro", name, line);
				break;
			} else if (x == -2) {
				jfgets(buf, sizeof(buf), fd);
				goto macroloop;
			}
			if (!m)
				break;

			/* Skip to end of key sequence */
			for (y = x; buf[y] != 0 && buf[y] != '\t' && buf[y] != '\n' && (buf[y] != ' ' || buf[y + 1] != ' '); ++y) ;
			buf[y] = 0;

			/* Add binding to context */
			if (kadd(cap, context, buf + x, m) == -1) {
				fprintf(stderr, "\n%s:%d: Bad key sequence '%s'", name, line, buf + x);
				err = 1;
			}
			break;
		}
	}
	/* close rc file */
	jfclose(fd);

	/* Print proper ending string */
	fprintf(stderr, "%cdone\n", err ? '\n' : ' ');

	/* 0 for success, 1 for syntax error */
	return (err);
}
