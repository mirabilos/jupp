/*-
 * Copyright © 2004, 2005, 2006, 2007, 2009, 2010, 2011, 2012,
 *	       2013, 2014, 2016, 2017, 2018
 *	mirabilos <m@mirbsd.org>
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
 *-
 * Compatibility and fully new utility functions for jupp.
 *
 * - jalloc: based on lalloc.c,v 1.26 from mksh
 * – ctime: based on mirtime from MirBSD libc; not leap second capable
 *   src/kern/include/mirtime.h,v 1.2 2011/11/20 23:40:11 tg Exp
 *   src/kern/c/mirtime.c,v 1.3 2011/11/20 23:40:10 tg Exp
 * – strlcpy, strlcat: pulled in via "strlfun.inc"
 * – popen, pclose: pulled in via "popen.inc"
 * - ustoc_{hex,oct}, ustol: parse integers
 */

#include "config.h"
#include "types.h"

__RCSID("$MirOS: contrib/code/jupp/compat.c,v 1.11 2018/06/26 20:49:33 tg Exp $");

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#ifdef MKSH_ALLOC_CATCH_UNDERRUNS
#include <sys/mman.h>
#include <err.h>
#endif
#include "tty.h"
#include "utils.h"

/* jalloc */

#define malloc_osi(sz)		malloc(sz)
#define realloc_osi(p,sz)	realloc((p), (sz))
#define free_osimalloc(p)	free(p)

#ifndef DEBUG
#define Sabrt void
#else
#define Sabrt const char *emsg
#endif
static void abrt(Sabrt) __attribute__((__noreturn__));

static void
abrt(Sabrt)
{
#ifdef DEBUG
	fputs(emsg, stderr);
#endif
#ifdef TEST
	write(2, "memory allocation error\n", 24);
#else
	ttabrt(0, "memory allocation error");
#endif
	/* try to get a coredump */
	abort();
}
#ifndef DEBUG
#define abrt(x) (abrt)()
#endif

/* build with CPPFLAGS+= -DUSE_REALLOC_MALLOC=0 on ancient systems */
#if defined(USE_REALLOC_MALLOC) && (USE_REALLOC_MALLOC == 0)
#define remalloc(p,n)	((p) == NULL ? malloc_osi(n) : realloc_osi((p), (n)))
#else
#define remalloc(p,n)	realloc_osi((p), (n))
#endif

#ifndef MKSH_ALLOC_CATCH_UNDERRUNS
#define ALLOC_ISUNALIGNED(p) (((size_t)(p)) % sizeof(struct jalloc_common))
#else
#define ALLOC_ISUNALIGNED(p) (((size_t)(p)) & 4095)
#undef remalloc
#undef free_osimalloc

static void
free_osimalloc(void *ptr)
{
	struct jalloc_item *lp = ptr;

	if (munmap(lp, lp->len))
		abrt("free_osimalloc");
}

static void *
remalloc(void *ptr, size_t size)
{
	struct jalloc_item *lp, *lold = ptr;

	size = (size + 4095) & ~(size_t)4095;

	if (lold && lold->len >= size)
		return (ptr);

	if ((lp = mmap(NULL, size, PROT_READ | PROT_WRITE,
	    MAP_ANON | MAP_PRIVATE, -1, (off_t)0)) == MAP_FAILED)
		abrt("remalloc: mmap");
	if (ALLOC_ISUNALIGNED(lp))
		abrt("remalloc: unaligned");
	if (mprotect(((char *)lp) + 4096, 4096, PROT_NONE))
		abrt("remalloc: mprotect");
	lp->len = size;

	if (lold) {
		memcpy(((char *)lp) + 8192, ((char *)lold) + 8192,
		    lold->len - 8192);
		if (munmap(lold, lold->len))
			abrt("remalloc: munmap");
	}

	return (lp);
}
#endif

void
jalloc_init(void)
{
#ifdef MKSH_ALLOC_CATCH_UNDERRUNS
	long pgsz;
	if ((pgsz = sysconf(_SC_PAGESIZE)) != 4096)
		errx(1, "fatal: pagesize %lu not 4096!", pgsz);
#endif
}

void *
jalloc(void *ptr, size_t nmemb, size_t siz)
{
	ALLOC_ITEM *lp;
	size_t usiz;

	if (notoktomul(nmemb, siz))
		abrt("jalloc nmemb*siz");
	usiz = nmemb * siz;
	if (notoktoadd(usiz, siz))
		abrt("jalloc usiz + terminator");
	usiz += siz;
	if (notoktoadd(usiz, sizeof(ALLOC_ITEM)))
		abrt("jalloc usiz + header");

	if (ptr) {
#ifdef DEBUG
		if (ALLOC_ISUNALIGNED(ptr))
			abrt("jalloc realloc unaligned");
#endif
		lp = jalloc_krnl(ptr);
	} else
		lp = NULL;

	if ((lp = remalloc(lp, usiz + sizeof(ALLOC_ITEM))) == NULL)
		abrt("jalloc remalloc failed");
#ifdef DEBUG
	if (ALLOC_ISUNALIGNED(lp))
		abrt("jalloc remalloc unaligned");
#endif
	if (!ptr)
		lp->ALLOC_INFO(elen) = 0;
	lp->ALLOC_INFO(esiz) = nmemb;
	return (jalloc_user(lp));
}

void
jfree(void *ptr)
{
	ALLOC_ITEM *lp;

	if (ptr != NULL) {
#ifdef DEBUG
		if (ALLOC_ISUNALIGNED(ptr))
			abrt("jalloc free unaligned");
#endif
		lp = jalloc_krnl(ptr);
		free_osimalloc(lp);
	}
}

/* strlcpy/strlcat */

#undef L_strlcat
#undef L_strlcpy
#ifndef HAVE_STRLCAT
#define L_strlcat
#endif
#ifndef HAVE_STRLCPY
#define L_strlcpy
#endif
#if defined(L_strlcat) || defined(L_strlcpy)
#undef __RCSID
#define __RCSID(x)		__IDSTRING(rcsid_strlfun_inc,x)
#define OUTSIDE_OF_LIBKERN
#include "strlfun.inc"
#endif

/* ctime */

#ifndef HAVE_CTIME
#ifdef TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# ifdef HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

typedef struct {
	int tm_sec;		/* seconds [0-60] */
	int tm_min;		/* minutes [0-59] */
	int tm_hour;		/* hours [0-23] */
	int tm_mday;		/* day of month [1-31] */
	int tm_mon;		/* month of year - 1 [0-11] */
	int tm_year;		/* year - 1900 */
	int tm_wday;		/* day of week (0 = sunday) */
} joe_tm;

static void joe_timet2tm(joe_tm *, const time_t *);

#if !HAVE_DECL_CTIME
char *ctime(const time_t *);
#endif

/* 302 / 1000 is log10(2.0) rounded up */
#define T_SIGNED(t)	(((t)-1) < 0)
#define T_MAXLEN(t)	((sizeof(t) * CHAR_BIT - T_SIGNED(t)) * 302 / 1000 + \
			    /* div.trunc. */ 1 + /* minus sign */ T_SIGNED(t))

static const char joe_days[7][4] = {
	"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};

static const char joe_months[12][4] = {
	"Jan", "Feb", "Mar", "Apr", "May", "Jun",
	"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

/*-
 * Dimensions for the buffer, example formats:
 * "Sun Jan  1 12:34:56 1234\n"
 * "Sat Dec 31 12:34:56     12345\n"
 *  <- 24 -----------------> + max.length of a year + NL + NUL
 */
static char joe_ctime_buf[24 + T_MAXLEN(time_t) + 2];

char *
ctime(const time_t *tp)
{
	int year;
	joe_tm tm;

	joe_timet2tm(&tm, tp);
	year = (int)(tm.tm_year + 1900);
	joe_snprintf_7(joe_ctime_buf, sizeof(joe_ctime_buf),
	    (year >= -999 && year <= 9999) ?
	    "%s %s %2d %02d:%02d:%02d %04d\n" :
	    "%s %s %2d %02d:%02d:%02d     %d\n",
	    joe_days[tm.tm_wday], joe_months[tm.tm_mon],
	    tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, year);
	return (joe_ctime_buf);
}

static void
joe_timet2tm(joe_tm *tm, const time_t *tp)
{
	int sec, day, mon;
	time_t y;

	/* convert to MJD */
	y = *tp;
	sec = (int)(y % 86400);
	y /= 86400;
	y += 40587;
	while (sec < 0) {
		--y;
		sec += 86400;
	}

	/* calculate 400-year cycle (y) and offset in it (day) */
	day = (int)(y % 146097);
	y /= 146097;

	/* add bias: 678881 = days between "convenient origin" and MJD 0 */
	/* convenient origin is Wed(3) 1 March 0(fictional)/-1(real) */
	day += 678881;
	/* recalculate offset in cycle (Gregorian Period) */
	y += day / 146097;
	day %= 146097;

	/* days in 400 years are cyclic, they have 20871 weeks */
	tm->tm_wday = (day + 3) % 7;

	/* calculate year from period, taking leap years into account */
	y *= 4;
	/* a long (Julian) century is at the end of each Gregorian Period */
	if (day == 146096) {
		y += 3;
		day = 36524;
	} else {
		y += day / 36524;
		day %= 36524;
	}
	y *= 25;
	y += day / 1461;
	day %= 1461;
	y *= 4;

	/* March to December, or January/February? */
	/* a leap year is at the end of each olympiad */
	if (day == 1460) {
		y += 3;
		day = 365;
	} else {
		y += day / 365;
		day %= 365;
	}

	/* count days and months from 1st March using fixed-point */
	day *= 10;
	mon = (day + 5) / 306;
	day = (day + 5) % 306;
	day /= 10;
	/* adjust for Jan/Feb offset */
	if (mon >= 10) {
		mon -= 10;
		++y;
	} else {
		mon += 2;
	}

	/* adjust for year 0(fictional) which did not exist */
	if (y < 1)
		--y;

	/* fill in the values still missing */
	tm->tm_sec = sec % 60;
	sec /= 60;
	tm->tm_min = sec % 60;
	tm->tm_hour = sec / 60;
	tm->tm_mday = day + 1;
	tm->tm_mon = mon;
	/*XXX truncate, for joe_snprintf doesn't know %lld portably */
	tm->tm_year = (int)(y - 1900);
}
#endif /* ndef HAVE_CTIME */

/* popen */

#ifndef HAVE_POPEN
#undef __RCSID
#define __RCSID(x)		__IDSTRING(rcsid_popen_inc,x)
#include "popen.inc"
#endif

/* utility stuff */

size_t
ustoc_hex(const void *us, int *dp, size_t lim)
{
	unsigned char c;
	const unsigned char *s = (const unsigned char *)us;
	int rv = 0, rounds = 0;

	while (rounds++ < 2 && lim-- > 0)
		if ((c = *s++) >= '0' && c <= '9')
			rv = (rv << 4) | (c & 15);
		else if ((c |= 0x20) >= 'a' && c <= 'f')
			rv = (rv << 4) | (c - 'a' + 10);
		else {
			--s;
			break;
		}

	*dp = rv;
	return (s - (const unsigned char *)us);
}

size_t
ustoc_oct(const void *us, int *dp, size_t lim)
{
	unsigned char c;
	const unsigned char *s = (const unsigned char *)us;
	int rv = 0, rounds = 0;

	while (rounds++ < 3 && lim-- > 0)
		if ((c = *s++) >= '0' && c <= '7')
			rv = (rv << 3) | (c & 7);
		else {
			--s;
			break;
		}

	*dp = rv;
	return (s - (const unsigned char *)us);
}

#define USTOL_MODEMASK	0x03

static const char ustol_wsp[] = "\t\x0B\x0C\r ";

long
ustol(void *us, void **dpp, int flags)
{
	unsigned char c, *s = (unsigned char *)us;
	unsigned long a = 0;
	unsigned char *sp;
	unsigned char neg = 0;

	if (flags & USTOL_LTRIM)
		while ((c = *s) && strchr(ustol_wsp, c))
			++s;

	switch (flags & USTOL_MODEMASK) {
	case USTOL_HEX:
		if (s[0] == '0' && (s[1] | 0x20) == 'x')
			s += 2;
		break;
	case USTOL_AUTO:
		if (s[0] != '0')
			flags |= USTOL_DEC;
		else if ((s[1] | 0x20) != 'x')
			flags |= USTOL_OCT;
		else {
			flags |= USTOL_HEX;
			s += 2;
		}
		break;
	}

	sp = s;
	switch (flags & USTOL_MODEMASK) {
	case USTOL_OCT:
		while ((c = *s++) >= '0' && c <= '7') {
			/* overflow check */
			if (a > (ULONG_MAX >> 3))
				goto err;
			/* accumulate trivially */
			a = (a << 3) | (c & 7);
		}
		break;
	case USTOL_HEX:
		while ((c = *s++) >= '0') {
			if (c <= '9')
				c &= 15;
			else if ((c |= 0x20) >= 'a' && c <= 'f')
				c = c - 'a' + 10;
			else
				break;
			/* overflow check */
			if (a > (ULONG_MAX >> 4))
				goto err;
			/* accumulate trivially */
			a = (a << 4) | c;
		}
		break;
	default:
		switch (*s) {
		case '-':
			neg = 1;
			/* FALLTHROUGH */
		case '+':
			sp = ++s;
		}
		while ((c = *s++) >= '0' && c <= '9') {
			c &= 15;
			if (a > (ULONG_MAX / 10))
				goto err;
			a *= 10;
			if (a > (ULONG_MAX - c))
				goto err;
			a += c;
		}
		if (neg) {
			if (a > (((unsigned long)(-(LONG_MIN + 1L))) + 1UL))
				goto err;
			a = -a;
		} else if (a > (unsigned long)LONG_MAX)
			goto err;
	}
	/* check we had at least one digit */
	if (--s == sp)
		goto err;

	if (flags & USTOL_RTRIM)
		while ((c = *s) && strchr(ustol_wsp, c))
			++s;

	/* don’t check for EOS, or arrived at EOS */
	if (!(flags & USTOL_EOS) || !*s)
		goto out;

 err:
	s = NULL;
	a = 0;
 out:
	if (dpp)
		*dpp = (void *)s;
	return ((long)a);
}

long
ustolb(void *us, void **dpp, long lower, long upper, int flags)
{
	void *dp;
	long rv;

	rv = ustol(us, &dp, flags);
	if (dp != NULL && (rv < lower || rv > upper)) {
		dp = NULL;
		rv = 0;
	}
	if (dpp)
		*dpp = dp;
	return (rv);
}
