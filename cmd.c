/*
 *	Command execution
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#define EXTERN_CMD_C
#include "config.h"
#include "types.h"

__RCSID("$MirOS: contrib/code/jupp/cmd.c,v 1.30 2018/01/18 21:59:11 tg Exp $");

#include <stdlib.h>
#include <string.h>

#include "b.h"
#include "bw.h"
#include "cmd.h"
#include "hash.h"
#include "help.h"
#include "kbd.h"
#include "macro.h"
#include "main.h"
#include "menu.h"
#include "path.h"
#include "poshist.h"
#include "pw.h"
#include "rc.h"
#include "tty.h"
#include "tw.h"
#include "ublock.h"
#include "uedit.h"
#include "uerror.h"
#include "ufile.h"
#include "uformat.h"
#include "uisrch.h"
#include "umath.h"
#include "undo.h"
#include "usearch.h"
#include "ushell.h"
#include "utag.h"
#include "utils.h"
#include "va.h"
#include "vs.h"
#include "utf8.h"
#include "w.h"

extern int marking;
extern int smode;
int dobeep = 0;
int uexecmd(BW *bw);

/* Command table */

static int
ubeep(void)
{
	ttputc(7);
	return 0;
}

extern char main_context[];
static int do_keymap(BW *bw, unsigned char *s, void *object, int *notify)
{
	KMAP *new_kmap;

	if (notify)
		*notify = 1;
	if (!*s || !(new_kmap = kmap_getcontext(s, 0))) {
		vsrm(s);
		return (-1);
	}
	if (bw->o.context != (unsigned char *)main_context)
		free(bw->o.context);
	bw->o.context = strcmp((char *)s, main_context) ?
	    (unsigned char *)strdup((char *)s) : (unsigned char *)main_context;
	rmkbd(bw->parent->kbd);
	bw->parent->kbd = mkkbd(new_kmap);
	joe_snprintf_1((char *)msgbuf, JOE_MSGBUFSIZE, "New keymap: %s", s);
	vsrm(s);
	msgnw(bw->parent, msgbuf);
	return (0);
}
static int ukeymap(BW *bw)
{
	if (wmkpw(bw->parent, UC "Name of keymap to switch to: ", NULL,
	    do_keymap, NULL, NULL, utypebw, NULL, NULL, locale_map)) {
		return (0);
	}
	return (-1);
}

static int unop(void)
{
	return (0);
}

#if !WANT_FORK
static int
unommu(BW *bw) {
	msgnw(bw->parent, UC "Sorry, not supported without MMU");
	return (-1);
}
#define ubknd	unommu
#define ubuild	unommu
#define urun	unommu
#endif

#define C(name,flag,func,m,arg,negarg) { UC name, UC negarg, func, m, flag, arg }
CMD cmds[] = {
C("abendjoe", TYPETW + TYPEPW + TYPEMENU + TYPEQW, uabendjoe, NULL, 0, NULL),
C("abort", TYPETW + TYPEPW + TYPEMENU + TYPEQW, uabort, NULL, 0, NULL),
C("abortbuf", TYPETW, uabortbuf, NULL, 0, NULL),
C("arg", TYPETW + TYPEPW + TYPEMENU + TYPEQW, uarg, NULL, 0, NULL),
C("ask", TYPETW + TYPEPW, uask, NULL, 0, NULL),
C("backs", TYPETW + TYPEPW + ECHKXCOL + EFIXXCOL + EMINOR + EKILL + EMOD, ubacks, NULL, 1, "delch"),
C("backsmenu", TYPEMENU, umbacks, NULL, 1, NULL),
C("backw", TYPETW + TYPEPW + ECHKXCOL + EFIXXCOL + EKILL + EMOD, ubackw, NULL, 1, "delw"),
C("beep", TYPETW + TYPEPW + TYPEMENU + TYPEQW, ubeep, NULL, 0, NULL),
C("begin_marking", TYPETW + TYPEPW, ubegin_marking, NULL, 0, NULL),
C("bknd", TYPETW + TYPEPW, ubknd, NULL, 0, NULL),
C("bkwdc", TYPETW + TYPEPW, ubkwdc, NULL, 1, "fwrdc"),
C("blkcpy", TYPETW + TYPEPW + EFIXXCOL + EMOD + EBLOCK, ublkcpy, NULL, 1, NULL),
C("blkdel", TYPETW + TYPEPW + EFIXXCOL + EKILL + EMOD + EBLOCK, ublkdel, NULL, 0, NULL),
C("blkmove", TYPETW + TYPEPW + EFIXXCOL + EMOD + EBLOCK, ublkmove, NULL, 0, NULL),
C("blksave", TYPETW + TYPEPW + EBLOCK, ublksave, NULL, 0, NULL),
C("bof", TYPETW + TYPEPW + EMOVE + EFIXXCOL, u_goto_bof, NULL, 0, NULL),
C("bofmenu", TYPEMENU, umbof, NULL, 0, NULL),
C("bol", TYPETW + TYPEPW + EFIXXCOL, u_goto_bol, NULL, 0, NULL),
C("bolmenu", TYPEMENU, umbol, NULL, 0, NULL),
C("bop", TYPETW + TYPEPW + EFIXXCOL, ubop, NULL, 1, "eop"),
C("bos", TYPETW + TYPEPW + EMOVE, ubos, NULL, 0, NULL),
C("bufed", TYPETW, ubufed, NULL, 0, NULL),
C("build", TYPETW + TYPEPW, ubuild, NULL, 0, NULL),
C("byte", TYPETW + TYPEPW, ubyte, NULL, 0, NULL),
C("cancel", TYPETW + TYPEPW + TYPEMENU + TYPEQW, ucancel, NULL, 0, NULL),
C("center", TYPETW + TYPEPW + EFIXXCOL + EMOD, ucenter, NULL, 1, NULL),
C("col", TYPETW + TYPEPW, ucol, NULL, 0, NULL),
C("complete", TYPEPW + EMINOR + EMOD, ucmplt, NULL, 0, NULL),
C("copy", TYPETW + TYPEPW, ucopy, NULL, 0, NULL),
C("crawll", TYPETW + TYPEPW, ucrawll, NULL, 1, "crawlr"),
C("crawlr", TYPETW + TYPEPW, ucrawlr, NULL, 1, "crawll"),
C("ctrl", TYPETW + TYPEPW + EMOD, uctrl, NULL, 0, NULL),
C("delbol", TYPETW + TYPEPW + EFIXXCOL + EKILL + EMOD, udelbl, NULL, 1, "deleol"),
C("delch", TYPETW + TYPEPW + ECHKXCOL + EFIXXCOL + EMINOR + EKILL + EMOD, udelch, NULL, 1, "backs"),
C("deleol", TYPETW + TYPEPW + EKILL + EMOD, udelel, NULL, 1, "delbol"),
C("dellin", TYPETW + TYPEPW + EFIXXCOL + EKILL + EMOD, udelln, NULL, 1, NULL),
C("delw", TYPETW + TYPEPW + EFIXXCOL + ECHKXCOL + EKILL + EMOD, u_word_delete, NULL, 1, "backw"),
C("dnarw", TYPETW + TYPEPW + EMOVE, udnarw, NULL, 1, "uparw"),
C("dnarwmenu", TYPEMENU, umdnarw, NULL, 1, "uparwmenu"),
C("dnslide", TYPETW + TYPEPW + TYPEMENU + TYPEQW + EMOVE, udnslide, NULL, 1, "upslide"),
C("drop", TYPETW + TYPEPW, udrop, NULL, 0, NULL),
C("dupw", TYPETW, uduptw, NULL, 0, NULL),
C("edit", TYPETW, uedit, NULL, 0, NULL),
C("eof", TYPETW + TYPEPW + EFIXXCOL + EMOVE, u_goto_eof, NULL, 0, NULL),
C("eofmenu", TYPEMENU, umeof, NULL, 0, NULL),
C("eol", TYPETW + TYPEPW + EFIXXCOL, u_goto_eol, NULL, 0, NULL),
C("eolmenu", TYPEMENU, umeol, NULL, 0, NULL),
C("eop", TYPETW + TYPEPW + EFIXXCOL, ueop, NULL, 1, "bop"),
C("execmd", TYPETW + TYPEPW, uexecmd, NULL, 0, NULL),
C("explode", TYPETW + TYPEPW + TYPEMENU + TYPEQW, uexpld, NULL, 0, NULL),
C("exsave", TYPETW + TYPEPW, uexsve, NULL, 0, NULL),
C("ffirst", TYPETW + TYPEPW, pffirst, NULL, 0, NULL),
C("filt", TYPETW + TYPEPW + EMOD + EBLOCK, ufilt, NULL, 0, NULL),
C("finish", TYPETW + TYPEPW + EMOD, ufinish, NULL, 1, NULL),
C("fmtblk", TYPETW + EMOD + EFIXXCOL + EBLOCK + ECHK0COL, ufmtblk, NULL, 1, NULL),
C("fnext", TYPETW + TYPEPW, pfnext, NULL, 1, NULL),
C("format", TYPETW + TYPEPW + EFIXXCOL + EMOD + ECHK0COL, uformat, NULL, 1, NULL),
C("fwrdc", TYPETW + TYPEPW, ufwrdc, NULL, 1, "bkwdc"),
C("gomark", TYPETW + TYPEPW + EMOVE, ugomark, NULL, 0, NULL),
C("groww", TYPETW, ugroww, NULL, 1, "shrinkw"),
C("help", TYPETW + TYPEPW + TYPEQW, u_help, NULL, 0, NULL),
C("helpcard", TYPETW + TYPEPW + TYPEQW, u_helpcard, NULL, 0, NULL),
C("hnext", TYPETW + TYPEPW + TYPEQW, u_help_next, NULL, 0, NULL),
C("home", TYPETW + TYPEPW + EFIXXCOL, uhome, NULL, 0, NULL),
C("hprev", TYPETW + TYPEPW + TYPEQW, u_help_prev, NULL, 0, NULL),
C("insc", TYPETW + TYPEPW + EFIXXCOL + EMOD, uinsc, NULL, 1, "delch"),
C("insf", TYPETW + TYPEPW + EMOD, uinsf, NULL, 0, NULL),
C("isrch", TYPETW + TYPEPW, uisrch, NULL, 0, NULL),
C("keymap", TYPETW + TYPEPW, ukeymap, NULL, 0, NULL),
C("killjoe", TYPETW + TYPEPW + TYPEMENU + TYPEQW, ukilljoe, NULL, 0, NULL),
C("killproc", TYPETW + TYPEPW, ukillpid, NULL, 0, NULL),
C("lindent", TYPETW + TYPEPW + EFIXXCOL + EMOD + EBLOCK, ulindent, NULL, 1, "rindent"),
C("line", TYPETW + TYPEPW, uline, NULL, 0, NULL),
C("lose", TYPETW + TYPEPW, ulose, NULL, 0, NULL),
C("lower", TYPETW + TYPEPW + EMOD + EBLOCK, ulower, NULL, 0, NULL),
C("ltarw", TYPETW + TYPEPW /* + EFIXXCOL + ECHKXCOL */, u_goto_left, NULL, 1, "rtarw"),
C("ltarwmenu", TYPEMENU, umltarw, NULL, 1, "rtarwmenu"),
C("macros", TYPETW + EFIXXCOL, umacros, NULL, 0, NULL),
C("markb", TYPETW + TYPEPW, umarkb, NULL, 0, NULL),
C("markk", TYPETW + TYPEPW, umarkk, NULL, 0, NULL),
C("markl", TYPETW + TYPEPW, umarkl, NULL, 0, NULL),
C("math", TYPETW + TYPEPW, umath, NULL, 0, NULL),
C("mathins", TYPETW + TYPEPW, umathins, NULL, 0, NULL),
C("mathres", TYPETW + TYPEPW, umathres, NULL, 0, NULL),
C("mode", TYPETW + TYPEPW + TYPEQW, umode, NULL, 0, NULL),
C("msg", TYPETW + TYPEPW + TYPEQW + TYPEMENU, umsg, NULL, 0, NULL),
C("nbuf", TYPETW, unbuf, NULL, 1, "pbuf"),
C("nedge", TYPETW + TYPEPW + EFIXXCOL, unedge, NULL, 1, "pedge"),
C("nextpos", TYPETW + TYPEPW + EFIXXCOL + EMID + EPOS, unextpos, NULL, 1, "prevpos"),
C("nextw", TYPETW + TYPEPW + TYPEMENU + TYPEQW, unextw, NULL, 1, "prevw"),
C("nextword", TYPETW + TYPEPW + EFIXXCOL, u_goto_next, NULL, 1, "prevword"),
C("nmark", TYPETW + TYPEPW, unmark, NULL, 0, NULL),
C("nop", TYPETW + TYPEPW + TYPEMENU + TYPEQW, unop, NULL, 0, NULL),
C("notmod", TYPETW, unotmod, NULL, 0, NULL),
C("nxterr", TYPETW, unxterr, NULL, 1, "prverr"),
C("open", TYPETW + TYPEPW + EFIXXCOL + EMOD, uopen, NULL, 1, "deleol"),
C("parserr", TYPETW, uparserr, NULL, 0, NULL),
C("pbuf", TYPETW, upbuf, NULL, 1, "nbuf"),
C("pedge", TYPETW + TYPEPW + EFIXXCOL, upedge, NULL, 1, "nedge"),
C("pgdn", TYPETW + TYPEPW + TYPEMENU + TYPEQW + EMOVE, upgdn, NULL, 1, "pgup"),
C("pgdnmenu", TYPEMENU, umpgdn, NULL, 1, "pgupmenu"),
C("pgup", TYPETW + TYPEPW + TYPEMENU + TYPEQW + EMOVE, upgup, NULL, 1, "pgdn"),
C("pgupmenu", TYPEMENU, umpgup, NULL, 1, "pgdnmenu"),
C("picokill", TYPETW + TYPEPW + EFIXXCOL + EKILL + EMOD, upicokill, NULL, 1, NULL),
C("play", TYPETW + TYPEPW + TYPEMENU + TYPEQW, uplay, NULL, 1, NULL),	/* EFIXX? */
C("pop", TYPETW + TYPEPW + TYPEMENU + TYPEQW, upop, NULL, 0, NULL),
C("prevpos", TYPETW + TYPEPW + EPOS + EMID + EFIXXCOL, uprevpos, NULL, 1, "nextpos"),
C("prevw", TYPETW + TYPEPW + TYPEMENU + TYPEQW, uprevw, NULL, 1, "nextw"),
C("prevword", TYPETW + TYPEPW + EFIXXCOL + ECHKXCOL, u_goto_prev, NULL, 1, "nextword"),
C("prverr", TYPETW, uprverr, NULL, 1, "nxterr"),
C("psh", TYPETW + TYPEPW + TYPEMENU + TYPEQW, upsh, NULL, 0, NULL),
C("qrepl", TYPETW + TYPEPW + EMOD, pqrepl, NULL, 0, NULL),
C("query", TYPETW + TYPEPW + TYPEMENU + TYPEQW, uquery, NULL, 0, NULL),
C("querysave", TYPETW, uquerysave, NULL, 0, NULL),
C("quote", TYPETW + TYPEPW + EMOD, uquote, NULL, 0, NULL),
C("quote8", TYPETW + TYPEPW + EMOD, uquote8, NULL, 0, NULL),
C("record", TYPETW + TYPEPW + TYPEMENU + TYPEQW, urecord, NULL, 0, NULL),
C("redo", TYPETW + TYPEPW + EFIXXCOL, uredo, NULL, 1, "undo"),
C("retype", TYPETW + TYPEPW + TYPEMENU + TYPEQW + ECHK0COL, uretyp, NULL, 0, NULL),
C("rfirst", TYPETW + TYPEPW, prfirst, NULL, 0, NULL),
C("rindent", TYPETW + TYPEPW + EFIXXCOL + EMOD + EBLOCK, urindent, NULL, 1, "lindent"),
C("rsrch", TYPETW + TYPEPW, ursrch, NULL, 0, NULL),
C("rtarw", TYPETW + TYPEPW /* + EFIXXCOL */, u_goto_right, NULL, 1, "ltarw"), /* EFIX removed for picture mode */
C("rtarwmenu", TYPEMENU, umrtarw, NULL, 1, "ltarwmenu"),
C("rtn", TYPETW + TYPEPW + TYPEMENU + TYPEQW + EMOD, urtn, NULL, 1, NULL),
C("run", TYPETW + TYPEPW, urun, NULL, 0, NULL),
C("rvmatch", TYPETW + TYPEPW + EFIXXCOL, urvmatch, NULL, 0, NULL),
C("save", TYPETW, usave, NULL, 0, NULL),
C("scratch", TYPETW + TYPEPW, uscratch, NULL, 0, NULL),
C("select", TYPETW + TYPEPW, uselect, NULL, 0, NULL),
C("setmark", TYPETW + TYPEPW, usetmark, NULL, 0, NULL),
C("shell", TYPETW + TYPEPW + TYPEMENU + TYPEQW, ushell, NULL, 0, NULL),
C("shrinkw", TYPETW, ushrnk, NULL, 1, "groww"),
C("splitw", TYPETW, usplitw, NULL, 0, NULL),
C("stat", TYPETW + TYPEPW, ustat_j, NULL, 0, NULL),
C("stop", TYPETW + TYPEPW + TYPEMENU + TYPEQW, ustop, NULL, 0, NULL),
C("sync", TYPETW + TYPEPW + TYPEMENU + TYPEQW, usync, NULL, 0, NULL),
C("swap", TYPETW + TYPEPW + EFIXXCOL, uswap, NULL, 0, NULL),
C("switch", TYPETW + TYPEPW, uswitch, NULL, 0, NULL),
C("tabmenu", TYPEMENU, umtab, NULL, 1, "ltarwmenu"),
C("tag", TYPETW + TYPEPW, utag, NULL, 0, NULL),
C("toggle_marking", TYPETW + TYPEPW, utoggle_marking, NULL, 0, NULL),
C("tomarkb", TYPETW + TYPEPW + EFIXXCOL + EBLOCK, utomarkb, NULL, 0, NULL),
C("tomarkbk", TYPETW + TYPEPW + EFIXXCOL + EBLOCK, utomarkbk, NULL, 0, NULL),
C("tomarkk", TYPETW + TYPEPW + EFIXXCOL + EBLOCK, utomarkk, NULL, 0, NULL),
C("tomatch", TYPETW + TYPEPW + EFIXXCOL, utomatch, NULL, 0, NULL),
C("tos", TYPETW + TYPEPW + EMOVE, utos, NULL, 0, NULL),
C("tw0", TYPETW + TYPEPW + TYPEQW + TYPEMENU, utw0, NULL, 0, NULL),
C("tw1", TYPETW + TYPEPW + TYPEQW + TYPEMENU, utw1, NULL, 0, NULL),
C("txt", TYPETW + TYPEPW, utxt, NULL, 0, NULL),
C("type", TYPETW + TYPEPW + TYPEQW + TYPEMENU + EMINOR + EMOD, utype, NULL, 1, "backs"),
C("uarg", TYPETW + TYPEPW + TYPEMENU + TYPEQW, uuarg, NULL, 0, NULL),
C("undo", TYPETW + TYPEPW + EFIXXCOL, uundo, NULL, 1, "redo"),
C("uparw", TYPETW + TYPEPW + EMOVE, uuparw, NULL, 1, "dnarw"),
C("uparwmenu", TYPEMENU, umuparw, NULL, 1, "dnarwmenu"),
C("upper", TYPETW + TYPEPW + EMOD + EBLOCK, uupper, NULL, 0, NULL),
C("upslide", TYPETW + TYPEPW + TYPEMENU + TYPEQW + EMOVE, uupslide, NULL, 1, "dnslide"),
C("yank", TYPETW + TYPEPW + EFIXXCOL + EMOD, uyank, NULL, 1, NULL),
C("yankpop", TYPETW + TYPEPW + EFIXXCOL + EMOD, uyankpop, NULL, 1, NULL),
C("yapp", TYPETW + TYPEPW + EKILL, uyapp, NULL, 0, NULL)
};

/* Execute a command n with key k */

int execmd(CMD *cmd, int k)
{
	BW *bw = maint->curwin->object.bw;
	int ret = -1;

	/* Send data to shell window: this is broken ^K ^H (help) sends its ^H to shell */
	if ((maint->curwin->watom->what & TYPETW) && bw->b->pid && piseof(bw->cursor) &&
	(k==3 || k==13 || k==8 || k==127 || k==4 || ((cmd->func==utype) && (k>=32) && (k<256)))) {
		unsigned char c = k;
		joe_write(bw->b->out, &c, 1);
		return 0;
	}

	if (cmd->m)
		return exmacro(cmd->m, 0);

	/* We don't execute if we have to fix the column position first
	 * (i.e., left arrow when cursor is in middle of nowhere) */
	if (cmd->flag & ECHKXCOL) {
		if (bw->o.hex)
			bw->cursor->xcol = piscol(bw->cursor);
		else if (bw->cursor->xcol != piscol(bw->cursor))
			goto skip;
	}

	/* Don't execute command if we're in wrong type of window */
	if (!(cmd->flag & maint->curwin->watom->what))
		goto skip;

	/* Complete selection for block commands */
	if ((cmd->flag & EBLOCK) && marking)
		utoggle_marking(maint->curwin->object.bw);

	if ((maint->curwin->watom->what & TYPETW) && bw->b->rdonly && (cmd->flag & EMOD)) {
		msgnw(bw->parent, UC "Read only");
		if (dobeep)
			ttputc(7);
		goto skip;
	}

	/* Execute command */
	ret = cmd->func(maint->curwin->object, k);

	if (smode)
		--smode;

	/* Don't update anything if we're going to leave */
	if (leave)
		return 0;

	/* cmd->func could have changed bw on us */
	bw = maint->curwin->object.bw;

	/* Maintain position history */
	/* If command was not a positioning command */
	if (!(cmd->flag & EPOS)
	    && (maint->curwin->watom->what & (TYPETW | TYPEPW)))
		afterpos();

	/* If command was not a movement */
	if (!(cmd->flag & (EMOVE | EPOS)) && (maint->curwin->watom->what & (TYPETW | TYPEPW)))
		aftermove(maint->curwin, bw->cursor);

	if (cmd->flag & EKILL)
		justkilled = 1;
	else
		justkilled = 0;

 skip:

	/* scroll screen to the left */
	if ((cmd->flag & ECHK0COL) && bw->offset != 0) {
		bw->offset = 0;
		updall();
	}

	/*
	 * Make displayed cursor column equal the actual cursor column
	 * for commands which aren't simple vertical movements
	 */
	if (cmd->flag & EFIXXCOL)
		bw->cursor->xcol = piscol(bw->cursor);

	/* Recenter cursor to middle of screen */
	if (cmd->flag & EMID) {
		int omid = mid;

		mid = 1;
		dofollows();
		mid = omid;
	}

	if (dobeep && ret)
		ttputc(7);
	return ret;
}

/* Return command table index for given command name */

HASH *cmdhash = NULL;

static void izcmds(void)
{
	int x;

	cmdhash = htmk(256);
	for (x = 0; x != sizeof(cmds) / sizeof(CMD); ++x)
		htadd(cmdhash, cmds[x].name, cmds + x);
}

CMD *findcmd(const unsigned char *s)
{
	if (!cmdhash)
		izcmds();
	return (CMD *) htfind(cmdhash, s);
}

void addcmd(const unsigned char *s, MACRO *m)
{
	CMD *cmd = malloc(sizeof(CMD));

	if (!cmdhash)
		izcmds();
	cmd->name = (unsigned char *)strdup((const char *)s);
	cmd->flag = 0;
	cmd->func = NULL;
	cmd->m = m;
	cmd->arg = 1;
	cmd->negarg = NULL;
	htadd(cmdhash, cmd->name, cmd);
}

static unsigned char **getcmds(void)
{
	unsigned char **s = vaensure(NULL, sizeof(cmds) / sizeof(CMD));
	int x;
	HENTRY *e;

	for (x = 0; x != cmdhash->len; ++x)
		for (e = cmdhash->tab[x]; e; e = e->next)
			s = vaadd(s, vsncpy(NULL, 0, sz(e->name)));
	vasort(s, aLen(s));
	return s;
}

/* Command line */

unsigned char **scmds = NULL;	/* Array of command names */

static int cmdcmplt(BW *bw)
{
	if (!scmds)
		scmds = getcmds();
	/*XXX simple_cmplt does p_goto_bol, better only to last comma */
	return simple_cmplt(bw, scmds);
}

static int docmd(BW *bw, unsigned char *s, void *object, int *notify)
{
	MACRO *mac;
	int ret = -1;

	if (s) {
		mac = mparse(NULL, s, &ret);
		if (ret < 0 || !mac)
			msgnw(bw->parent, UC "No such command");
		else {
			ret = exmacro(mac, 1);
			rmmacro(mac);
		}
	}
	vsrm(s);	/* allocated in pw.c::rtnpw() */
	if (notify)
		*notify = 1;
	return ret;
}

B *cmdhist = NULL;

int uexecmd(BW *bw)
{
	if (wmkpw(bw->parent, UC "cmd: ", &cmdhist, docmd, UC "cmd", NULL, cmdcmplt, NULL, NULL, locale_map)) {
		return 0;
	} else {
		return -1;
	}
}

/*
 * Show help screen at a specific card
 */
static int do_helpcard(BASE *base, unsigned char *s, void *object, int *notify)
{
	struct help *new_help;

	if (notify)
		*notify = 1;
	if (!*s) {
		vsrm(s);
		while (help_actual->prev != NULL)
			/* find the first help entry */
			help_actual = help_actual->prev;
		help_off(base->parent->t);
		return (0);
	}
	if ((new_help = find_context_help(s)) != NULL) {
		vsrm(s);
		help_actual = new_help;
		return (help_on(base->parent->t));
	}
	vsrm(s);
	return (-1);
}
int u_helpcard(BASE *base)
{
	if (wmkpw(base->parent, UC "Name of help card to show: ", NULL,
	    do_helpcard, NULL, NULL, utypebw, NULL, NULL, locale_map)) {
		return (0);
	}
	return (-1);
}
