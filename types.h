#ifndef JUPP_TYPES_H
#define JUPP_TYPES_H

#ifdef EXTERN
__IDSTRING(rcsid_types_h, "$MirOS: contrib/code/jupp/types.h,v 1.39 2020/04/07 11:56:41 tg Exp $");
#endif

/*-
 * Copyright © 2004, 2005, 2006, 2007, 2008, 2011, 2012, 2013,
 *	       2014, 2016, 2017, 2018, 2020
 *	Thorsten “mirabilos” Glaser <tg@mirbsd.org>
 *
 * Provided that these terms and disclaimer and all copyright notices
 * are retained or reproduced in an accompanying document, permission
 * is granted to deal in this work without restriction, including un‐
 * limited rights to use, publicly perform, distribute, sell, modify,
 * merge, give away, or sublicence.
 *
 * This work is provided “AS IS” and WITHOUT WARRANTY of any kind, to
 * the utmost extent permitted by applicable law, neither express nor
 * implied; without malicious intent or gross negligence. In no event
 * may a licensor, author or contributor be held liable for indirect,
 * direct, other damage, loss, or other issues arising in any way out
 * of dealing in the work, even if advised of the possibility of such
 * damage or existence of a defect, except proven that it results out
 * of said person’s immediate fault when using the work as intended.
 */

/* Prefix to make string constants unsigned */
#define UC (const unsigned char *)
#define US (unsigned char *)

#define LINK(type) struct { type *next; type *prev; }

#ifdef SMALL
#define stdsiz		4096
#else
#define stdsiz		8192
#endif
#define FITHEIGHT	4		/* Minimum text window height */
#define LINCOLS		6
#define NPROC		8		/* Number of processes we keep track of */
#define UNDOKEEP	100
#define INC		16		/* Pages to allocate each time */

#define TYPETW		0x0100
#define TYPEPW		0x0200
#define TYPEMENU	0x0800
#define TYPEQW		0x1000

/* polymorph function pointers, which do not use compiler type checking */
#ifndef GCC_Wstrict_prototypes
typedef int jpoly_int();
typedef void jpoly_void();
#else
/* same content as above, but system header */
#include <jupp.tmp.h>
#endif

struct jalloc_common {
	/*XXX these must be size_t not int */
	/*
	 * size part: number of elements that can be fit,
	 * not counting the terminator or space needed for the header
	 */
	int esiz;
	/*
	 * length part: number of elements currently in the array,
	 * not counting the terminator
	 */
	int elen;
};

#ifdef MKSH_ALLOC_CATCH_UNDERRUNS
struct jalloc_item {
	struct jalloc_common enfo;
	size_t len;
	char dummy[8192 - sizeof(struct jalloc_common) - sizeof(size_t)];
};
#define ALLOC_INFO(f)	enfo.f
#define ALLOC_ITEM	struct jalloc_item
#else
#define ALLOC_INFO(f)	f
#define ALLOC_ITEM	struct jalloc_common
#endif

#define jalloc_krnl(i)	((ALLOC_ITEM *)((char *)(i) - sizeof(ALLOC_ITEM)))
#define jalloc_user(i)	((void *)((char *)(i) + sizeof(ALLOC_ITEM)))
#define jalloc_siz(i)	(jalloc_krnl(i)->ALLOC_INFO(esiz))
#define jalloc_len(i)	(jalloc_krnl(i)->ALLOC_INFO(elen))

#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif
#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif

#ifndef SIZE_MAX
#ifdef SIZE_T_MAX
#define SIZE_MAX	SIZE_T_MAX
#else
#define SIZE_MAX	((size_t)-1)
#endif
#endif

#define notok2mul(max,val,c)	(((val) != 0) && ((c) != 0) && \
				    (((max) / (c)) < (val)))
#define notok2add(max,val,c)	((val) > ((max) - (c)))
#define notoktomul(val,cnst)	notok2mul(SIZE_MAX, (val), (cnst))
#define notoktoadd(val,cnst)	notok2add(SIZE_MAX, (val), (cnst))

void jalloc_init(void);
void *jalloc(void *, size_t, size_t);
void jfree(void *);

#define ralloc(nmemb,size)	(notoktomul(nmemb, size) ? NULL : \
				    malloc((nmemb) * (size)))

typedef struct header H;
typedef struct buffer B;
typedef struct point P;
typedef struct options OPTIONS;
typedef struct macro MACRO;
typedef struct cmd CMD;
typedef struct hentry HENTRY;
typedef struct hash HASH;
typedef struct kmap KMAP;
typedef struct kbd KBD;
typedef struct key KEY;
typedef struct watom WATOM;
typedef struct screen SCREEN;
typedef struct window W;
typedef struct base BASE;
typedef struct bw BW;
typedef struct menu MENU;
typedef struct scrn SCRN;
typedef struct cap CAP;
typedef struct pw PW;
typedef struct stditem STDITEM;
typedef struct query QW;
typedef struct tw TW;
typedef struct irec IREC;
typedef struct undo UNDO;
typedef struct undorec UNDOREC;
typedef struct search SRCH;
typedef struct srchrec SRCHREC;
typedef struct vpage VPAGE;
typedef struct vfile VFILE;

/* window.object* */
typedef union {
	BASE *base;
	MENU *menu;
	BW *bw;
	QW *qw;
} jobject;

struct header {
	LINK(H)	link;		/* LINK ??? */
	long	seg;		/* ??? */
	int	hole;		/* ??? */
	int	ehole;		/* ??? */
	int	nlines;		/* ??? */
};

struct point {
	LINK(P)	link;		/* ?LINK ??? */

	B	*b;		/* ?B ??? */
	int	ofst;		/* ??? */
	unsigned char	*ptr;	/* ??? */
	H	*hdr;		/* ?H ??? */

	long	byte;		/* ??? */
	long	line;		/* ??? */
	long	col;		/* current column */
	long	xcol;		/* ??? */
	int	valcol;		/* bool: is col valid? */
	int	end;		/* ??? */

	P	**owner;	/* ??? */
};

struct options {
	OPTIONS	*next;
	unsigned char	*name_regex;
	unsigned char	*contents_regex;
	int	overtype;
	int	lmargin;
	int	rmargin;
	int	autoindent;
	int	wordwrap;
	int	tab;
	int	indentc;
	int	istep;
	unsigned char *context;
	const unsigned char *lmsg;
	const unsigned char *rmsg;
	char	*hmsg;
	int	linums;
	int	readonly;
	int	french;
	int	spaces;
	int	crlf;
	int	highlight;	/* Set to enable highlighting */
	unsigned char *syntax_name;	/* Name of syntax to use */
	struct high_syntax *syntax;	/* Syntax for highlighting (load_dfa() from syntax_name happens in setopt()) */
	unsigned char *map_name;	/* Name of character set */
	union charmap *charmap;	/* Character set */
	int	smarthome;	/* Set for smart home key */
	int	indentfirst;	/* Smart home goes to indentation point first */
	int	smartbacks;	/* Set for smart backspace key */
	int	purify;		/* Purify indentation */
	int	picture;	/* Picture mode */
	MACRO	*mnew;		/* Macro to execute for new files */
	MACRO	*mold;		/* Macro to execute for existing files */
	MACRO	*msnew;		/* Macro to execute before saving new files */
	MACRO	*msold;		/* Macro to execute before saving existing files */
	int	vispace;	/* Set to make spaces visible */
	int	hex;		/* Hex edit mode */
};

struct macro {
	int	k;		/* Keycode */
	int	arg;		/* Repeat argument */
	CMD	*cmd;		/* Command address */
	int	n;		/* Number of steps */
	int	size;		/* Malloc size of steps */
	MACRO	**steps;	/* Block */
};

struct recmac {
	struct recmac *next;
	int	n;
	MACRO	*m;
};


/* Command entry */

struct cmd {
	const unsigned char *name;	/* Command name */
	const unsigned char *negarg;	/* Command to use if arg was negative */
	jpoly_int *func;	/* Function bound to name */
	MACRO	*m;		/* Macro bound to name */
	unsigned int flag;	/* Execution flags */
	int	arg;		/* 0= arg is meaningless, 1= ok */
};



struct buffer {
	LINK(B)	link;
	P	*bof;
	P	*eof;
	unsigned char	*name;
	long    mod_time;	/* Last modification time for file */
	int	orphan;
	int	count;
	int	changed;
	int	backup;
	void	*undo;
	P	*marks[11];	/* Bookmarks */
	OPTIONS	o;		/* Options */
	P	*oldcur;	/* Last cursor position before orphaning */
	P	*oldtop;	/* Last top screen position before orphaning */
	int	rdonly;		/* Set for read-only */
	int	internal;	/* Set for internal buffers */
	int	scratch;	/* Set for scratch buffers */
	int	er;		/* Error code when file was loaded */
	pid_t	pid;		/* Process id */
	int	out;		/* fd to write to process */
};


struct hentry {
	const unsigned char *name;
	HENTRY	*next;
	void	*val;
};

struct hash {
	int	len;
	HENTRY	**tab;
};


struct help {
	struct help	*prev;		/* previous help screen */
	struct help	*next;		/* nex help screen */
	unsigned char	*name;		/* context name for context sensitive help */
	unsigned char	*text;		/* help text with attributes */
	unsigned int	lines;		/* number of lines */
};

/* A key binding */
struct key {
	int	k;			/* Flag: 0=binding, 1=submap */
	union {
		void	*bind;		/* What key is bound to */
		KMAP	*submap;	/* Sub KMAP address (for prefix keys) */
	} value;
};

/* A map of keycode (octet) to command/sub-map bindings */
struct kmap {
	KEY	keys[256];	/* KEYs */
};

/** A keyboard handler **/
struct kbd {
	KMAP	*curmap;	/* Current keymap */
	KMAP	*topmap;	/* Top-level keymap */
	int	seq[16];	/* Current sequence of keys */
	int	x;		/* What we're up to */
};


struct watom {
	const unsigned char *context;	/* Context name */
	void (*disp)(jobject, int);	/* Display window */
	void (*follow)(jobject);	/* Display window */
	int (*abort)(jobject);		/* Common user functions */
	int (*rtn)(jobject);
	int (*type)(jobject, int);
					/* Called when… */
	void (*resize)(jobject, int, int);	/* window changed size */
	void (*move)(jobject, int, int);	/* window moved */
	void (*ins)(BW *, B *, long, long, int);	/* on line insertions */
	void (*del)(BW *, B *, long, long, int);	/* on line deletions */
	int	what;		/* Type of this thing */
};

struct screen {
	SCRN	*t;		/* Screen data on this screen is output to */

	int	wind;		/* Number of help lines on this screen */

	W	*topwin;	/* Top-most window showing on screen */
	W	*curwin;	/* Window cursor is in */

	int	w, h;		/* Width and height of this screen */
};

struct window {
	LINK(W)	link;		/* Linked list of windows in order they
				   appear on the screen */

	SCREEN	*t;		/* Screen this thing is on */

	int	x, y, w, h;	/* Position and size of window */
				/* Currently, x = 0, w = width of screen. */
				/* y == -1 if window is not on screen */

	int	ny, nh;		/* Temporary values for wfit */

	int	reqh;		/* Requested new height or 0 for same */
				/* This is an argument for wfit */

	int	fixed;		/* If this is zero, use 'hh'.  If not, this
				   is a fixed size window and this variable
				   gives its height */

	int	hh;		/* Height window would be on a screen with
				   1000 lines.  When the screen size changes
				   this is used to calculate the window's
				   real height */

	W	*win;		/* Window this one operates on */
	W	*main;		/* Main window of this family */
	W	*orgwin;	/* Window where space from this window came */
	int	curx, cury;	/* Cursor position within window */
	KBD	*kbd;		/* Keyboard handler for this window */
	WATOM	*watom;		/* The type of this window */
	jobject	object;		/* Object which inherits this */

	const unsigned char *msgt;	/* Message at top of window */
	const unsigned char *msgb;	/* Message at bottom of window */
	const unsigned char *huh;	/* Name of window for context sensitive hlp */
	int	*notify;	/* Address of kill notification flag */
};

/* Anything which goes in window.object must start like this: */
struct base {
	W	*parent;
};

struct bw {
	W	*parent;
	B	*b;
	P	*top;
	P	*cursor;
	long	offset;
	SCREEN	*t;
	int	h, w, x, y;

	OPTIONS	o;
	void	*object;

	int	linums;
	int	top_changed;	/* Top changed */
};

struct menu {
	W	*parent;	/* Window we're in */
	unsigned char	**list;		/* List of items */
	int	top;		/* First item on screen */
	int	cursor;		/* Item cursor is on */
	int	width;		/* Width of widest item, up to 'w' max */
	int	perline;	/* Number of items on each line */
	int	nitems;		/* No. items in list */
	int	saved_co;	/* Saved #columns of screen */
	SCREEN	*t;		/* Screen we're on */
	int	h, w, x, y;
	jpoly_int *abrt;	/* Abort callback function */
	jpoly_int *func;	/* Return callback function */
	jpoly_int *backs;	/* Backspace callback function */
	void	*object;
};

struct s_hentry {
	int	next;
	int	loc;
};

/* Each terminal has one of these */
struct scrn {
	CAP	*cap;		/* Termcap/Terminfo data */

	int	li;		/* Screen height */
	int	co;		/* Screen width */

	const unsigned char *ti;	/* Initialisation string */
	const unsigned char *cl;	/* Home and clear screen... really an
					   init. string */
	const unsigned char *cd;	/* Clear to end of screen */
	const unsigned char *te;	/* Restoration string */

	int	haz;		/* Terminal can't print ~s */
	int	os;		/* Terminal overstrikes */
	int	eo;		/* Can use blank to erase even if os */
	int	ul;		/* _ overstrikes */
	int	am;		/* Terminal has autowrap, but not magicwrap */
	int	xn;		/* Terminal has magicwrap */

	const unsigned char *so;	/* Enter standout (inverse) mode */
	const unsigned char *se;	/* Exit standout mode */

	const unsigned char *us;	/* Enter underline mode */
	const unsigned char *ue;	/* Exit underline mode */
	const unsigned char *uc;	/* Single time underline character */

	int	ms;		/* Ok to move when in standout/underline mode */

	const unsigned char *mb;	/* Enter blinking mode */
	const unsigned char *md;	/* Enter bold mode */
	const unsigned char *mh;	/* Enter dim mode */
	const unsigned char *mr;	/* Enter inverse mode */
	const unsigned char *me;	/* Exit above modes */

	const unsigned char *Sb;	/* Set background color */
	const unsigned char *Sf;	/* Set foregrond color */
	int	ut;		/* Screen erases with background color */

	int	da, db;		/* Extra lines exist above, below */
	const unsigned char *al, *dl, *AL, *DL;	/* Insert/delete lines */
	const unsigned char *cs;		/* Set scrolling region */
	int	rr;		/* Set for scrolling region relative addressing */
	const unsigned char *sf, *SF, *sr, *SR;	/* Scroll */

	const unsigned char *dm, *dc, *DC, *ed;	/* Delete characters */
	const unsigned char *im, *ic, *IC, *ip, *ei;	/* Insert characters */
	int	mi;		/* Set if ok to move while in insert mode */

	const unsigned char *bs;	/* Move cursor left 1 */
	int	cbs;
	const unsigned char *lf;	/* Move cursor down 1 */
	int	clf;
	const unsigned char *up;	/* Move cursor up 1 */
	int	cup;
	const unsigned char *nd;	/* Move cursor right 1 */

	const unsigned char *ta;	/* Move cursor to next tab stop */
	int	cta;
	const unsigned char *bt;	/* Move cursor to previous tab stop */
	int	cbt;
	int	tw;			/* Tab width */

	const unsigned char *ho;	/* Home cursor to upper left */
	int	cho;
	const unsigned char *ll;	/* Home cursor to lower left */
	int	cll;
	const unsigned char *cr;	/* Move cursor to left edge */
	int	ccr;
	const unsigned char *RI;	/* Move cursor right n */
	int	cRI;
	const unsigned char *LE;	/* Move cursor left n */
	int	cLE;
	const unsigned char *UP;	/* Move cursor up n */
	int	cUP;
	const unsigned char *DO;	/* Move cursor down n */
	int	cDO;
	const unsigned char *ch;	/* Set cursor column */
	int	cch;
	const unsigned char *cv;	/* Set cursor row */
	int	ccv;
	const unsigned char *cV;	/* Goto beginning of specified line */
	int	ccV;
	const unsigned char *cm;	/* Set cursor row and column */
	int	ccm;

	const unsigned char *ce;	/* Clear to end of line */
	int	cce;

	/* Basic abilities */
	int	scroll;		/* Set to use scrolling */
	int	insdel;		/* Set to use insert/delete within line */

	/* Current state of terminal */
	int	*scrn;		/* Characters on screen */
	int	*attr;		/* Attributes on screen */
	int	x, y;		/* Current cursor position (-1 for unknown) */
	int	top, bot;	/* Current scrolling region */
	int	attrib;		/* Current character attributes */
	int	ins;		/* Set if we're in insert mode */

	int	*updtab;	/* Dirty lines table */
	int	*syntab;
	int	avattr;		/* Bits set for available attributes */
	int	*sary;		/* Scroll buffer array */

	int	*compose;	/* Line compose buffer */
	int	*ofst;		/* stuff for magic */
	struct s_hentry	*htab;
	struct s_hentry	*ary;
};


struct sortentry {
	unsigned char	*name;
	unsigned char	*value;
};

struct cap {
	unsigned char	*tbuf;		/* Termcap entry loaded here */

	struct sortentry *sort;	/* Pointers to each capability stored in here */
	int	sortlen;	/* Number of capabilities */

	unsigned char	*abuf;		/* For terminfo compatible version */
	unsigned char	*abufp;

	int	div;		/* tenths of MS per char */
	int	baud;		/* Baud rate */
	const unsigned char *pad;	/* Padding string or NULL to use NUL */
	int	(*out)(int);	/* character output routine */
	int	dopadding;	/* Set if pad characters should be used */
	const char *paste_on;	/* Enable bracketed paste mode */
	const char *paste_off;	/* Disable bracketed paste mode */
};


struct pw {
	jpoly_int *pfunc;	/* Func which gets called when RTN is hit */
	jpoly_int *abrt;	/* Func which gets called when window is aborted */
	jpoly_int *tab;		/* Func which gets called when TAB is hit */
	unsigned char *prompt;	/* Prompt string */
	int	promptlen;	/* Width of prompt string */
	int	promptofst;	/* Prompt scroll offset */
	B	*hist;		/* History buffer */
	void	*object;	/* Object */
};

struct stditem {
	LINK(STDITEM)	link;
};

struct query {
	W	*parent;	/* Window we're in */
	jpoly_int *func;	/* Func. which gets called when key is hit */
	jpoly_int *abrt;
	void	*object;
	unsigned char	*prompt;	/* Prompt string */
	int	promptlen;	/* Width of prompt string */
	int	promptofst;	/* Prompt scroll offset */
};


typedef struct mpx MPX;
struct mpx {
	int	ackfd;		/* Packetizer response descriptor */
	int	kpid;		/* Packetizer process id */
	int	pid;		/* Client process id */
	jpoly_void *func;	/* Function to call when read occures */
	void	*object;	/* First arg to pass to function */
	jpoly_void *die;	/* Function: call when client dies or closes */
	void	*dieobj;
};


struct tw {
	unsigned char	*stalin;	/* Status line info */
	unsigned char	*staright;
	int	staon;		/* Set if status line was on */
	long	prevline;	/* Previous cursor line number */
	int	changed;	/* Previous changed value */
	B	*prev_b;	/* Previous buffer (we need to update status line on nbuf/pbuf) */
};

struct irec {
	LINK(IREC)	link;
	int	what;		/* 0 repeat, >0 append n chars */
	long	start;		/* Cursor search position */
	long	disp;		/* Original cursor position */
	int	wrap_flag;	/* Wrap flag */
};

struct isrch {
	IREC	irecs;		/* Linked list of positions */
	unsigned char *pattern;	/* Search pattern string */
	unsigned char *prompt;	/* Prompt (usually same as pattern unless utf-8/byte conversion) */
	int	ofst;		/* Offset in pattern past prompt */
	int	dir;		/* 0=fwrd, 1=bkwd */
	int	quote;		/* Set to quote next char */
};


struct undorec {
	LINK(UNDOREC)	link;
	UNDOREC	*unit;
	int	min;
	int	changed;	/* Status of modified flag before this record */
	long	where;		/* Buffer address of this record */
	long	len;		/* Length of insert or delete */
	int	del;		/* Set if this is a delete */
	B	*big;		/* Set to buffer containing a large amount of deleted data */
	unsigned char *small;	/* Set to malloc block containg a small amount of deleted data */
};

struct undo {
	LINK(UNDO)	link;
	B	*b;
	int	nrecs;
	UNDOREC	recs;
	UNDOREC	*ptr;
	UNDOREC	*first;
	UNDOREC	*last;
};

struct srchrec {
	LINK(SRCHREC)	link;	/* Linked list of search & replace locations */
	int	yn;		/* Did we replace? */
	int	wrap_flag;	/* Did we wrap? */
	long	addr;		/* Where we were */
};

struct search {
	unsigned char	*pattern;	/* Search pattern */
	unsigned char	*replacement;	/* Replacement string */
	int	backwards;	/* Set if search should go backwards */
	int	ignore;		/* Set if we should ignore case */
	int	repeat;		/* Set with repeat count (or -1 for no repeat count) */
	int	replace;	/* Set if this is search & replace */
	int	rest;		/* Set to do remainder of search & replace w/o query */
	unsigned char	*entire;	/* Entire matched string */
	unsigned char	*pieces[26];	/* Pieces of the matched string */
	int	flg;		/* Set after prompted for first replace */
	SRCHREC	recs;		/* Search & replace position history */
	P	*markb, *markk;	/* Original marks */
	P	*wrap_p;	/* Wrap point */
	int	wrap_flag;	/* Set if we've wrapped */
	int	valid;		/* Set if original marks are a valid block */
	long	addr;		/* Addr of last replacement or -1 for none */
	int	block_restrict;	/* Search restricted to marked block */
};



/* Page header */

struct vpage {
	VPAGE	*next;		/* Next page with same hash value */
	VFILE	*vfile;		/* Owner vfile */
	long	addr;		/* Address of this page */
	int	count;		/* Reference count */
	int	dirty;		/* Set if page changed */
	unsigned char	*data;		/* The data in the page */
};

/* File structure */

struct vfile {
	LINK(VFILE)	link;	/* Doubly linked list of vfiles */
	long	size;		/* Number of bytes in physical file */
	long	alloc;		/* Number of bytes allocated to file */
	int	fd;		/* Physical file */
	int	writeable;	/* Set if we can write */
	unsigned char	*name;		/* File name.  0 if unnamed */
	int	flags;		/* Set if this is only a temporary file */

	/* For array I/O */
	unsigned char	*vpage1;	/* Page address */
	long	addr;		/* File address of above page */

	/* For stream I/O */
	unsigned char	*bufp;		/* Buffer pointer */
	unsigned char	*vpage;		/* Buffer pointer points in here */
	int	left;		/* Space left in bufp */
	int	lv;		/* Amount of append space at end of buffer */
};

#endif
