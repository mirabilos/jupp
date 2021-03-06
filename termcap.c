/*
 *	TERMCAP/TERMINFO database interface
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#include "config.h"
#include "types.h"

__RCSID("$MirOS: contrib/code/jupp/termcap.c,v 1.28 2020/03/27 06:30:16 tg Exp $");

#include <sys/stat.h>
#include <stdlib.h>

#include "blocks.h"
#include "path.h"
#include "termcap.h"
#include "utils.h"
#include "va.h"
#include "vs.h"

int dopadding = 0;
unsigned char *joeterm = NULL;

/* Default termcap entry */

static const unsigned char defentry[] = "\
:co#80:li#25:am:\
:ho=\\E[H:cm=\\E[%i%d;%dH:cV=\\E[%i%dH:\
:up=\\E[A:UP=\\E[%dA:DO=\\E[%dB:nd=\\E[C:RI=\\E[%dC:LE=\\E[%dD:\
:cd=\\E[J:ce=\\E[K:cl=\\E[H\\E[J:\
:so=\\E[7m:se=\\E[m:us=\\E[4m:ue=\\E[m:\
:mb=\\E[5m:md=\\E[1m:mh=\\E[2m:me=\\E[m:\
:ku=\\E[A:kd=\\E[B:kl=\\E[D:kr=\\E[C:\
:al=\\E[L:AL=\\E[%dL:dl=\\E[M:DL=\\E[%dM:\
:ic=\\E[@:IC=\\E[%d@:dc=\\E[P:DC=\\E[%dP:\
";

/* Return true if termcap line matches name */

static int
match(const unsigned char *s, const unsigned char *name)
{
	if (s[0] == 0 || s[0] == '#')
		return 0;
	do {
		int x;

		for (x = 0; s[x] == name[x] && name[x] && s[x]; ++x) ;
		if (name[x] == 0 && (s[x] == ':' || s[x] == '|'))
			return 1;
		while (s[x] != ':' && s[x] != '|' && s[x])
			++x;
		s += x + 1;
	} while (s[-1] == '|');
	return 0;
}

/* Find termcap entry in a file */

static unsigned char *
lfind(unsigned char *s, int pos, FILE *fd, const unsigned char *name)
{
	int c, x;

	if (!s)
		s = vsmk(1024);
 loop:
	while (c = getc(fd), c == ' ' || c == '\t' || c == '#')
		do {
			c = getc(fd);
		} while (c != -1 && c != '\n');
	if (c == -1)
		return s = vstrunc(s, pos);
	ungetc(c, fd);
	s = vstrunc(s, x = pos);
	while (1) {
		c = getc(fd);
		if (c == -1 || c == '\n')
			if (x != pos && s[x - 1] == '\\') {
				--x;
				if (!match(s + pos, name))
					goto loop;
				else
					break;
			} else if (!match(s + pos, name))
				goto loop;
			else
				return vstrunc(s, x);
		else if (c == '\r')
			/* nothing */;
		else {
			s = vsset(s, x, c);
			++x;
		}
	}
	while (c = getc(fd), c != -1)
		if (c == '\n')
			if (s[x - 1] == '\\')
				--x;
			else
				break;
		else if (c == '\r')
			/* nothing */;
		else {
			s = vsset(s, x, c);
			++x;
		}
	s = vstrunc(s, x);
	return s;
}

/* Lookup termcap entry in index */

static off_t
findidx(FILE *file, const unsigned char *name)
{
	unsigned char buf[80];
	off_t addr = 0;

	while (fgets((char *)buf, 80, file)) {
		int x = 0, flg = 0, c, y;

		do {
			for (y = x; buf[y] && buf[y] != ' ' && buf[y] != '\n'; ++y) ;
			c = buf[y];
			buf[y] = 0;
			if (c == '\n' || !c)
				addr += ustol(buf + x, NULL, USTOL_HEX);
			else if (!strcmp(buf + x, name))
				flg = 1;
			x = y + 1;
		} while (c && c != '\n');
		if (flg)
			return addr;
	}
	return 0;
}

/* Load termcap entry */

CAP *
getcap(unsigned char *name, unsigned int baud, int (*out)(int))
{
	CAP *cap;
	FILE *f, *f1;
	off_t idx;
	int x, y, c, z, ti;
	unsigned char *tp, *pp, *qq, *namebuf, **npbuf, *idxname;
	int sortsiz;

	if (!name && !(name = joeterm) && !(name = (unsigned char *)getenv("TERM")))
		return NULL;
	cap = malloc(sizeof(CAP));
	cap->tbuf = vsmk(4096);
	cap->abuf = NULL;
	cap->sort = NULL;
	cap->paste_on = NULL;
	cap->paste_off = NULL;

	if (!strcmp(name, "xterm-xfree86")) {
		cap->paste_on = "\033[?2004h";
		cap->paste_off = "\033[?2004l";
	}

#ifdef TERMINFO
	cap->abuf = malloc(4096);
	cap->abufp = cap->abuf;
	if (tgetent((char *)cap->tbuf, (char *)name) == 1)
		return setcap(cap, baud, out);
	else {
		free(cap->abuf);
		cap->abuf = NULL;
	}
#endif

	name = vsncpy(NULL, 0, sz(name));
	cap->sort = malloc(sizeof(struct sortentry) * (sortsiz = 64));

	cap->sortlen = 0;

	tp = (unsigned char *)getenv("TERMCAP");

	if (tp && tp[0] == '/')
		namebuf = vsncpy(NULL, 0, sz(tp));
	else {
		if (tp)
			cap->tbuf = vsncpy(sv(cap->tbuf), sz(tp));
		if ((tp = (unsigned char *)getenv("TERMPATH")))
			namebuf = vsncpy(NULL, 0, sz(tp));
		else {
			if ((tp = (unsigned char *)getenv("HOME"))) {
				namebuf = vsncpy(NULL, 0, sz(tp));
				namebuf = vsadd(namebuf, '/');
			} else
				namebuf = NULL;
			namebuf = vsncpy(sv(namebuf), sc(".termcap "));
			if (has_JOERC &&
			    vsscan(sz(get_JOERC), sc("\t :")) == ~0) {
				namebuf = vsncpy(sv(namebuf), sz(get_JOERC));
				namebuf = vsncpy(sv(namebuf), sc("termcap "));
			}
			namebuf = vsncpy(sv(namebuf), sc("/etc/termcap"));
		}
	}

	npbuf = vawords(NULL, sv(namebuf), sc("\t :"));
	vsrm(namebuf);

	y = 0;
	ti = 0;

	if (match(cap->tbuf, name))
		goto checktc;

	cap->tbuf = vstrunc(cap->tbuf, 0);

 nextfile:
	if (!npbuf[y]) {
		fprintf(stderr, "Couldn't load termcap entry.  Using ansi default\n");
		ti = 0;
		cap->tbuf = vsncpy(cap->tbuf, 0, sc(defentry));
		goto checktc;
	}
	idx = 0;
	idxname = vsncpy(NULL, 0, sz(npbuf[y]));
	idxname = vsncpy(idxname, sLEN(idxname), sc(".idx"));
	f1 = fopen((char *)(npbuf[y]), "r");
	++y;
	if (!f1)
		goto nextfile;
	f = fopen((char *)idxname, "r");
	if (f) {
		struct stat buf, buf1;

		if (fstat(fileno(f), &buf) || fstat(fileno(f1), &buf1))
			fprintf(stderr, "cannot stat termcap index\n");
		else if (buf.st_mtime > buf1.st_mtime)
			idx = findidx(f, name);
		else
			fprintf(stderr, "%s is out of date\n", idxname);
		fclose(f);
	}
	vsrm(idxname);
#ifdef HAVE_FSEEKO
	fseeko(f1, idx, 0);
#else
	/* ugh. SOL! */
	fseek(f1, (long)idx, 0);
#endif
	cap->tbuf = lfind(cap->tbuf, ti, f1, name);
	fclose(f1);
	if (sLEN(cap->tbuf) == ti)
		goto nextfile;

 checktc:
	x = sLEN(cap->tbuf);
	do {
		cap->tbuf[x] = 0;
		while (x && cap->tbuf[--x] != ':')
			/* nothing */;
	} while (x && (!cap->tbuf[x + 1] || cap->tbuf[x + 1] == ':'));

	if (cap->tbuf[x + 1] == 't' && cap->tbuf[x + 2] == 'c' && cap->tbuf[x + 3] == '=') {
		name = vsncpy(NULL, 0, sz(cap->tbuf + x + 4));
		cap->tbuf[x] = 0;
		cap->tbuf[x + 1] = 0;
		ti = x + 1;
		sLen(cap->tbuf) = x + 1;
		if (y)
			--y;
		goto nextfile;
	}

 doline:
	pp = cap->tbuf + ti;

	/* Process line at pp */

 loop:
	while (*pp && *pp != ':')
		++pp;
	if (*pp) {
		int q;

		*pp++ = 0;
 loop1:
		if (pp[0] == ' ' || pp[0] == '\t')
			goto loop;
		for (q = 0; pp[q] && pp[q] != '#' && pp[q] != '=' && pp[q] != '@' && pp[q] != ':'; ++q) ;
		qq = pp;
		c = pp[q];
		pp[q] = 0;
		if (c)
			pp += q + 1;
		else
			pp += q;

		x = 0;
		y = cap->sortlen;
		z = -1;
		if (!y)
			goto in;
		while (z != (x + y) / 2) {
			int found;

			z = (x + y) / 2;
			found = strcmp(qq, cap->sort[z].name);
			if(found > 0) {
				x = z;
			} else if(found < 0) {
				y = z;
			} else {
				if (c == '@')
					mmove(cap->sort + z, cap->sort + z + 1, (cap->sortlen-- - (z + 1)) * sizeof(struct sortentry));

				else if (c && c != ':')
					cap->sort[z].value = qq + q + 1;
				else
					cap->sort[z].value = NULL;
				if (c == ':')
					goto loop1;
				else
					goto loop;
			}
		}
 in:
		if (cap->sortlen == sortsiz)
			cap->sort = realloc(cap->sort, (sortsiz += 32) * sizeof(struct sortentry));
		mmove(cap->sort + y + 1, cap->sort + y, (cap->sortlen++ - y) * sizeof(struct sortentry));

		cap->sort[y].name = qq;
		if (c && c != ':')
			cap->sort[y].value = qq + q + 1;
		else
			cap->sort[y].value = NULL;
		if (c == ':')
			goto loop1;
		else
			goto loop;
	}

	if (ti) {
		for (--ti; ti; --ti)
			if (!cap->tbuf[ti - 1])
				break;
		goto doline;
	}

	varm(npbuf);
	vsrm(name);

	cap->pad = jgetstr(cap, UC "pc");
	if (dopadding)
		cap->dopadding = 1;
	else
		cap->dopadding = 0;

/* show sorted entries
	for(x=0;x!=cap->sortlen;++x)
		printf("%s = %s\n",cap->sort[x].name,cap->sort[x].value);
*/
	return setcap(cap, baud, out);
}

static struct sortentry *
findcap(CAP *cap, const unsigned char *name)
{
	int x, y, z;
	int found;

	x = 0;
	y = cap->sortlen;
	z = -1;
	while (z != (x + y) / 2) {
		z = (x + y) / 2;
		found = strcmp(name, cap->sort[z].name);
		if (found > 0)
			x = z;
		else if (found < 0)
			y = z;
		else
			return cap->sort + z;
	}
	return NULL;
}

CAP *
setcap(CAP *cap, unsigned int baud, int (*out)(int))
{
	cap->baud = baud;
	cap->div = 100000 / baud;
	cap->out = out;
	return cap;
}

#ifdef TERMINFO
/* const-dirty curses interface to the terminfo capability database */
union compat_hack {
	char *rw;
	const unsigned char *ro;
};
#endif

int
getflag(CAP *cap, const unsigned char *name)
{
#ifdef TERMINFO
	if (cap->abuf) {
		union compat_hack hack;

		hack.ro = name;
		return tgetflag(hack.rw);
	}
#endif
	return findcap(cap, name) != NULL;
}

const unsigned char *
jgetstr(CAP *cap, const unsigned char *name)
{
	struct sortentry *s;

#ifdef TERMINFO
	if (cap->abuf) {
		union compat_hack hack;

		hack.ro = name;
		return (const unsigned char *)tgetstr(hack.rw,
		    (char **)&cap->abufp);
	}
#endif
	s = findcap(cap, name);
	if (s)
		return s->value;
	else
		return NULL;
}

int
getnum(CAP *cap, const unsigned char *name)
{
	struct sortentry *s;

#ifdef TERMINFO
	if (cap->abuf) {
		union compat_hack hack;

		hack.ro = name;
		return tgetnum(hack.rw);
	}
#endif
	s = findcap(cap, name);
	if (s && s->value)
		return atoi((const char *)(s->value));
	return -1;
}

void rmcap(CAP *cap)
{
	vsrm(cap->tbuf);
	if (cap->abuf)
		free(cap->abuf);
	if (cap->sort)
		free(cap->sort);
	free(cap);
}

static unsigned char
escape(const unsigned char **s)
{
	unsigned char c = *(*s)++;
	int i;

	if (c == '^' && **s)
		if (**s != '?')
			return 037 & *(*s)++;
		else {
			(*s)++;
			return 127;
		}
	else if (c == '\\' && **s)
		switch (c = *((*s)++)) {
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
			(*s)--;
			*s += ustoc_oct(*s, &i, USTOC_MAX);
			return i;
		case 'e':
		case 'E':
			return 27;
		case 'n':
		case 'l':
			return 10;
		case 'r':
			return 13;
		case 't':
			return 9;
		case 'b':
			return 8;
		case 'f':
			return 12;
		case 's':
			return 32;
		default:
			return c;
	} else
		return c;
}

void
texec(CAP *cap, const unsigned char *s, int l, int a0, int a1, int a2, int a3)
{
	int c, tenth = 0, x;
	int args[4];
	int *a = args;
	int *vars = NULL;

	/* Do nothing if there is no string */
	if (!s)
		return;

#ifdef TERMINFO
	if (cap->abuf) {
		unsigned char *aa;

		aa = (unsigned char *)tgoto((const char *)s, a1, a0);
		tputs((char *)aa, l, cap->out);
		return;
	}
#endif

	/* Copy args into array (yuk) */
	args[0] = a0;
	args[1] = a1;
	args[2] = a2;
	args[3] = a3;

	/* Get tenths of MS of padding needed */
	while (*s >= '0' && *s <= '9')
		tenth = tenth * 10 + *s++ - '0';
	tenth *= 10;
	if (*s == '.') {
		++s;
		tenth += *s++ - '0';
	}

	/* Check if we have to multiply by number of lines */
	if (*s == '*') {
		++s;
		tenth *= l;
	}

	/* Output string */
	while ((c = *s++) != '\0')
		if (c == '%' && *s) {
			switch (x = a[0], c = escape(&s)) {
			case 'C':
				if (x >= 96) {
					cap->out((unsigned char)(x / 96));
					x %= 96;
				}
				/* FALLTHROUGH */
			case '+':
				if (*s)
					x += escape(&s);
				/* FALLTHROUGH */
			case '.':
				cap->out((unsigned char)x);
				++a;
				break;
			case 'd':
				if (x < 10)
					goto one;
				/* FALLTHROUGH */
			case '2':
				if (x < 100)
					goto two;
				/* FALLTHROUGH */
			case '3':
				c = '0';
				while (x >= 100) {
					++c;
					x -= 100;
				}
				cap->out((unsigned char)c);
 two:
				c = '0';
				while (x >= 10) {
					++c;
					x -= 10;
				}
				cap->out((unsigned char)c);
 one:
				cap->out((unsigned char)('0' + x));
				++a;
				break;
			case 'r':
				a[0] = a[1];
				a[1] = x;
				break;
			case 'i':
				++a[0];
				++a[1];
				break;
			case 'n':
				a[0] ^= 0140;
				a[1] ^= 0140;
				break;
			case 'm':
				a[0] ^= 0177;
				a[1] ^= 0177;
				break;
			case 'f':
				++a;
				break;
			case 'b':
				--a;
				break;
			case 'a':
				x = s[2];
				if (s[1] == 'p')
					x = a[x - 0100];
				switch (*s) {
				case '+':
					a[0] += x;
					break;
				case '-':
					a[0] -= x;
					break;
				case '*':
					a[0] *= x;
					break;
				case '/':
					a[0] /= x;
					break;
				case '%':
					a[0] %= x;
					break;
				case 'l':
					if (vars)
						a[0] = vars[x];
					break;
				case 's':
					if (!vars)
						vars = calloc(256, sizeof(int));
					if (vars)
						vars[x] = a[0];
					break;
				default:
					a[0] = x;
				}
				s += 3;
				break;
			case 'D':
				a[0] = a[0] - 2 * (a[0] & 15);
				break;
			case 'B':
				a[0] = 16 * (a[0] / 10) + a[0] % 10;
				break;
			case '>':
				if (a[0] > escape(&s))
					a[0] += escape(&s);
				else
					escape(&s);
				/* FALLTHROUGH */
			default:
				cap->out((unsigned char)'%');
				cap->out((unsigned char)c);
			}
		} else {
			--s;
			cap->out((unsigned char)escape(&s));
		}

/* Output padding characters */
	if (cap->dopadding) {
		if (cap->pad)
			while (tenth >= cap->div)
				for (s = cap->pad; *s; ++s) {
					cap->out((unsigned char)(*s));
					tenth -= cap->div;
				}
		else
			while (tenth >= cap->div) {
				cap->out(0);
				tenth -= cap->div;
			}
	}

	free(vars);
}

static int total;

static int
cst(int c)
{
	++total;
	return (c);
}

int tcost(CAP *cap, const unsigned char *s, int l, int a0, int a1, int a2, int a3)
{
	int (*out)(int) = cap->out;

	if (!s)
		return 10000;
	total = 0;
	cap->out = cst;
	texec(cap, s, l, a0, a1, a2, a3);
	cap->out = out;
	return total;
}

static unsigned char *ssp;
static int
cpl(int c)
{
	ssp = vsadd(ssp, (unsigned char)c);
	return (c);
}

unsigned char *
tcompile(CAP *cap, const unsigned char *s, int a0, int a1, int a2, int a3)
{
	int (*out)(int) = cap->out;
	int divider = cap->div;

	if (!s)
		return NULL;
	cap->out = cpl;
	cap->div = 10000;
	ssp = vsmk(10);
	texec(cap, s, 0, a0, a1, a2, a3);
	cap->out = out;
	cap->div = divider;
	return ssp;
}
