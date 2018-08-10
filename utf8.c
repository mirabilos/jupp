/*
 *	UTF-8 Utilities
 *	Copyright
 *		(C) 2004 Joseph H. Allen
 *		(c) 2004, 2006, 2011, 2013, 2014, 2017, 2018 mirabilos
 *
 *	This file is part of JOE (Joe's Own Editor)
 */

#include "config.h"
#include "types.h"

__RCSID("$MirOS: contrib/code/jupp/utf8.c,v 1.25 2018/08/10 02:53:45 tg Exp $");

#include <stdlib.h>
#include <string.h>

#ifdef __CYGWIN__
#include <cygwin/version.h>
#endif

#ifdef __MirBSD__
#include <sys/param.h>
#endif

#undef USE_CODEPAGE
#undef USE_LOCALE
#if defined(HAVE_SETLOCALE) && defined(HAVE_NL_LANGINFO)
#define USE_LOCALE
#endif

/* Cygwin before 1.7.2 did not have locale support */
#if defined(CYGWIN_VERSION_API_MAJOR) && (CYGWIN_VERSION_API_MAJOR < 1) && \
    defined(CYGWIN_VERSION_API_MINOR) && (CYGWIN_VERSION_API_MINOR < 222)
#define USE_CODEPAGE
#undef USE_LOCALE
#endif

/* OpenBSD, ekkoBSD and old MirOS do not have real locale support */
#if defined(__OpenBSD__) && (!defined(MirBSD) || (MirBSD < 0x09A0))
#undef USE_LOCALE
#endif

#ifdef USE_LOCALE
#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif
#ifdef HAVE_LANGINFO_H
#include <langinfo.h>
#endif
#endif

#ifndef CODESET
#undef USE_LOCALE
#endif

#ifdef USE_LOCALE
#undef USE_CODEPAGE
#endif

#include "rc.h"
#include "charmap.h"

/* UTF-8 Encoder
 *
 * c is a UCS character.
 * buf is 7 byte buffer: UTF-8 encoded character is written to this followed by a NUL terminator
 * returns length (not including terminator).
 */

int utf8_encode(unsigned char *buf,int c)
{
	if (c < 0x80) {
		buf[0] = c;
		buf[1] = 0;
		return 1;
	} else if(c < 0x800) {
		buf[0] = (0xc0|(c>>6));
		buf[1] = (0x80|(c&0x3F));
		buf[2] = 0;
		return 2;
	} else if(c < 0x10000) {
		buf[0] = (0xe0|(c>>12));
		buf[1] = (0x80|((c>>6)&0x3f));
		buf[2] = (0x80|((c)&0x3f));
		buf[3] = 0;
		return 3;
	} else if(c < 0x200000) {
		buf[0] = (0xf0|(c>>18));
		buf[1] = (0x80|((c>>12)&0x3f));
		buf[2] = (0x80|((c>>6)&0x3f));
		buf[3] = (0x80|((c)&0x3f));
		buf[4] = 0;
		return 4;
	} else if(c < 0x4000000) {
		buf[0] = (0xf8|(c>>24));
		buf[1] = (0x80|((c>>18)&0x3f));
		buf[2] = (0x80|((c>>12)&0x3f));
		buf[3] = (0x80|((c>>6)&0x3f));
		buf[4] = (0x80|((c)&0x3f));
		buf[5] = 0;
		return 5;
	} else {
		buf[0] = (0xfC|(c>>30));
		buf[1] = (0x80|((c>>24)&0x3f));
		buf[2] = (0x80|((c>>18)&0x3f));
		buf[3] = (0x80|((c>>12)&0x3f));
		buf[4] = (0x80|((c>>6)&0x3f));
		buf[5] = (0x80|((c)&0x3f));
		buf[6] = 0;
		return 6;
	}
}

/* UTF-8 Decoder
 *
 * Returns 0 - 7FFFFFFF: decoded character
 *                   -1: byte accepted, nothing decoded yet
 *                   -2: illegal continuation byte or sequence
 *                   -3: illegal start byte
 */

int
utf8_decode(struct utf8_sm *utf8_sm, unsigned char c)
{
	if (utf8_sm->state) {
		utf8_sm->buf[utf8_sm->ptr] = c;
		if ((c ^= 0x80) < 0x40) {
			/* trail byte */
			++utf8_sm->ptr;
			utf8_sm->accu = (utf8_sm->accu << 6) | c;
			if (--utf8_sm->state)
				return (-1);
			if (utf8_sm->accu >= utf8_sm->minv)
				return (utf8_sm->accu);
			/* reject non-minimal-encoded sequence */
			--utf8_sm->ptr;
		}
		utf8_sm->state = 0;
		return (-2);
	} else if (c < 0x80) {
		utf8_sm->accu = c; /* known to be in [0; 127] */
		utf8_sm->state = 0;
	} else if (c < 0xC2) {
 ilchar:
		utf8_init(utf8_sm);
		return (-3);
	} else if (c < 0xE0) {
		utf8_sm->accu = c & 0x1F;
		utf8_sm->state = 1;
	} else if (c < 0xF0) {
		utf8_sm->accu = c & 0x0F;
		utf8_sm->state = 2;
	} else if (c < 0xF8) {
		utf8_sm->accu = c & 0x07;
		utf8_sm->state = 3;
	} else if (c < 0xFC) {
		utf8_sm->accu = c & 0x03;
		utf8_sm->state = 4;
	} else if (c < 0xFE) {
		utf8_sm->accu = c & 0x01;
		utf8_sm->state = 5;
	} else
		goto ilchar;

	utf8_sm->minv = 1 << (5 * utf8_sm->state + 1);
	utf8_sm->buf[0] = c;
	utf8_sm->ptr = 1;

	if (!utf8_sm->state)
		/* ASCII */
		return (utf8_sm->accu);

	/* lead byte */
	utf8_sm->minv = 1 << (5 * utf8_sm->state + 1);
	return (-1);
}

/* Initialise state machine */

void utf8_init(struct utf8_sm *utf8_sm)
{
	utf8_sm->ptr = 0;
	utf8_sm->state = 0;
}

/* Decode an entire string */

int utf8_decode_string(unsigned char *s)
{
	struct utf8_sm sm;
	int x;
	int c = 0;
	utf8_init(&sm);
	for(x=0;s[x];++x)
		c = utf8_decode(&sm,s[x]);
	return c;
}

/* Decode and advance */

int utf8_decode_fwrd(unsigned char **p,int *plen)
{
	struct utf8_sm sm;
	unsigned char *s = *p;
	int len = *plen;
	int c = -2;

	utf8_init(&sm);

	while (len) {
		--len;
		c = utf8_decode(&sm,*s++);
		if (c >= 0)
			break;
	}

	*plen = len;
	*p = s;

	return c;
}

/* Initialize locale for JOE */

#ifdef USE_CODEPAGE
extern unsigned int cygwin32_get_cp(void);
#endif

struct charmap *locale_map;
			/* Character map of terminal */
struct charmap *utf8_map;
			/* Handy character map for UTF-8 */

void
joe_locale(void)
{
	unsigned char *s;

	s=(unsigned char *)getenv("JOECHARMAP");
	locale_map = find_charmap(s);
#if !defined(USE_LOCALE)
	if (!locale_map) {
		s=(unsigned char *)getenv("LC_ALL");
		if (!s) {
			s=(unsigned char *)getenv("LC_CTYPE");
			if (!s) {
				s=(unsigned char *)getenv("LANG");
			}
		}
#ifdef USE_CODEPAGE
		/* if LC_* are unset, use codepage */
		if (!s) {
			char buf[16];

			joe_snprintf_1(buf, sizeof(buf), "cp%u", cygwin32_get_cp());
			locale_map = find_charmap(buf);
		}
#endif
	}
#endif

#ifdef USE_LOCALE
	if (!locale_map) {
		setlocale(LC_ALL,"");
		locale_map = find_charmap((const void *)nl_langinfo(CODESET));
	}
#else
	if (!locale_map && s) {
		unsigned char *t, *tt;

		if ((t = strrchr(s, '.')) != NULL) {
			if ((tt = strchr(++t, '@')) != NULL)
				*tt = '\0';
			locale_map = find_charmap(t);
		}
		if (!locale_map)
			locale_map = find_charmap(s);
	}
#endif
	if (!locale_map)
		locale_map = find_charmap(UC "ascii");
	utf8_map = find_charmap(UC "utf-8");

#ifndef TEST
#ifdef defutf8
	fdefault.charmap = utf8_map;
#else
	fdefault.charmap = locale_map;
#endif
	pdefault.charmap = locale_map;
#endif
}

void to_utf8(struct charmap *map,unsigned char *s,int c)
{
	int d = to_uni(map,c);

	if (d==-1)
		utf8_encode(s,'?');
	else
		utf8_encode(s,d);
}

int from_utf8(struct charmap *map,unsigned char *s)
{
	int d = utf8_decode_string(s);
	int c = from_uni(map,d);
	if (c==-1)
		return '?';
	else
		return c;
}
