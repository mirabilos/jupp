/* $MirOS: contrib/code/jupp/utf8.c,v 1.13 2017/01/10 21:04:31 tg Exp $ */
/*
 *	UTF-8 Utilities
 *	Copyright
 *		(C) 2004 Joseph H. Allen
 *		(c) 2004, 2006 Thorsten Glaser
 *
 *	This file is part of JOE (Joe's Own Editor)
 */

#include "config.h"
#include "types.h"

#include <stdio.h>
#include <string.h>

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#ifdef __MirBSD__
#include <sys/param.h>
#endif

#undef USE_LOCALE
#if defined(HAVE_SETLOCALE) && defined(HAVE_NL_LANGINFO)
#define USE_LOCALE
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

#include "rc.h"
#include "utf8.h"
#include "charmap.h"

/* UTF-8 Encoder
 *
 * c is unicode character.
 * buf is 7 byte buffer- utf-8 coded character is written to this followed by a 0 termination.
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
 *                   -1: character accepted, nothing decoded yet.
 *                   -2: incomplete sequence
 *                   -3: no sequence started, but character is between 128 - 191, 254 or 255
 */

int utf8_decode(struct utf8_sm *utf8_sm,unsigned char c)
{
	if (utf8_sm->state) {
		if ((c&0xC0)==0x80) {
			utf8_sm->buf[utf8_sm->ptr++] = c;
			--utf8_sm->state;
			utf8_sm->accu = ((utf8_sm->accu<<6)|(c&0x3F));
			if(!utf8_sm->state)
				return utf8_sm->accu;
		} else {
			utf8_sm->state = 0;
			return -2;
		}
	} else if ((c&0xE0)==0xC0) {
		/* 192 - 223 */
		utf8_sm->buf[0] = c;
		utf8_sm->ptr = 1;
		utf8_sm->state = 1;
		utf8_sm->accu = (c&0x1F);
	} else if ((c&0xF0)==0xE0) {
		/* 224 - 239 */
		utf8_sm->buf[0] = c;
		utf8_sm->ptr = 1;
		utf8_sm->state = 2;
		utf8_sm->accu = (c&0x0F);
	} else if ((c&0xF8)==0xF0) {
		/* 240 - 247 */
		utf8_sm->buf[0] = c;
		utf8_sm->ptr = 1;
		utf8_sm->state = 3;
		utf8_sm->accu = (c&0x07);
	} else if ((c&0xFC)==0xF8) {
		/* 248 - 251 */
		utf8_sm->buf[0] = c;
		utf8_sm->ptr = 1;
		utf8_sm->state = 4;
		utf8_sm->accu = (c&0x03);
	} else if ((c&0xFE)==0xFC) {
		/* 252 - 253 */
		utf8_sm->buf[0] = c;
		utf8_sm->ptr = 1;
		utf8_sm->state = 5;
		utf8_sm->accu = (c&0x01);
	} else if ((c&0x80)==0x00) {
		/* 0 - 127 */
		utf8_sm->buf[0] = c;
		utf8_sm->ptr = 1;
		utf8_sm->state = 0;
		return c;
	} else {
		/* 128 - 191, 254, 255 */
		utf8_sm->ptr = 0;
		utf8_sm->state = 0;
		return -3;
	}
	return -1;
}

/* Initialize state machine */

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

unsigned char *codeset;	/* Codeset of terminal */

struct charmap *locale_map;
			/* Character map of terminal */
struct charmap *utf8_map;
			/* Handy character map for UTF-8 */

void
joe_locale(void)
{
#if !defined(USE_LOCALE)
	unsigned char *s, *t;


	s=(unsigned char *)getenv("LC_ALL");
	if (!s) {
		s=(unsigned char *)getenv("LC_CTYPE");
		if (!s) {
			s=(unsigned char *)getenv("LANG");
		}
	}
#endif

#ifdef USE_LOCALE
	setlocale(LC_ALL,"");
	codeset = (unsigned char *)strdup(nl_langinfo(CODESET));

	locale_map = find_charmap(codeset);
#else
	if (s == NULL) {
		locale_map = NULL;
	} else {
		if ((t = strrchr(s, '.')) != NULL) {
			unsigned char *tt;

			if ((tt = strchr(++t, '@')) != NULL)
				*tt = '\0';
			locale_map = find_charmap(t);
		}
		if (locale_map == NULL)
			locale_map = find_charmap(s);
	}
#endif
	if (!locale_map)
		locale_map = find_charmap(US "ascii");
	utf8_map = find_charmap(US "utf-8");

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
