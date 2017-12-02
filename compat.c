/*-
 * Copyright © 2004, 2005, 2006, 2007, 2011, 2012
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
 *-
 * Compatibility functions for jupp.
 *
 * – ctime: based on mirtime from MirBSD libc; not leap second capable
 *   src/kern/include/mirtime.h,v 1.2 2011/11/20 23:40:11 tg Exp
 *   src/kern/c/mirtime.c,v 1.3 2011/11/20 23:40:10 tg Exp
 * – strlcpy, strlcat: pulled in via "strlfun.inc"
 * – popen, pclose: pulled in via "popen.inc"
 */

#include "config.h"
#include "types.h"

__RCSID("$MirOS: contrib/code/jupp/compat.c,v 1.5 2017/12/02 02:07:25 tg Exp $");

#undef L_strlcat
#undef L_strlcpy
#ifndef HAVE_STRLCAT
#define L_strlcat
#endif
#ifndef HAVE_STRLCPY
#define L_strlcpy
#endif
#if defined(L_strlcat) || defined(L_strlcpy)
#define OUTSIDE_OF_LIBKERN
#include "strlfun.inc"
#endif

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

#include <limits.h>

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

#ifndef HAVE_POPEN
#include "popen.inc"
#endif
