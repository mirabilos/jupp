/*
 *	Various utilities
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *		(C) 2001 Marek 'Marx' Grac
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#include "config.h"

__RCSID("$MirOS: contrib/code/jupp/utils.c,v 1.11 2018/11/11 18:15:39 tg Exp $");

#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

#include "charmap.h"
#include "blocks.h"
#include "utils.h"

/*
 * return minimum/maximum of two numbers
 */
unsigned int uns_min(unsigned int a, unsigned int b)
{
	return a < b ? a : b;
}

signed int int_min(signed int a, signed int b)
{
	return a < b ? a : b;
}

signed long int long_max(signed long int a, signed long int b)
{
	return a > b ? a : b;
}

signed long int long_min(signed long int a, signed long int b)
{
	return a < b ? a : b;
}

/* Versions of 'read' and 'write' which automatically retry when interrupted */
ssize_t
joe_read(int fd, void *buf, size_t size)
{
	ssize_t rt;

	do {
		rt = read(fd, buf, size);
	} while (rt < 0 && errno == EINTR);
	return rt;
}

ssize_t
joe_write(int fd, const void *buf, size_t size)
{
	ssize_t rt;

	do {
		rt = write(fd, buf, size);
	} while (rt < 0 && errno == EINTR);
	return rt;
}

/* Similarily, read and write an exact amount (up to EOF) */
ssize_t
joe_readex(int fd, void *buf_, size_t size)
{
	unsigned char *buf = buf_;
	ssize_t rv = 0, z;

	while (size) {
		if ((z = read(fd, buf, size)) < 0) {
			if (errno == EINTR)
				continue;
			return (rv ? /* fucked up since we got some */ -2 : -1);
		}
		if (z == 0)
			break;
		rv += z;
		buf += z;
		size -= z;
	}
	return (rv);
}

#if 0 /* unused */
ssize_t
joe_writex(int fd, const void *buf_, size_t size)
{
	const unsigned char *buf = buf_;
	ssize_t rv = 0, z;

	while (size) {
		if ((z = write(fd, buf, size)) < 0) {
			if (errno == EINTR)
				continue;
			return (rv ? /* fucked up since we got some */ -2 : -1);
		}
		rv += z;
		buf += z;
		size -= z;
	}
	return (rv);
}
#endif

#ifndef SIG_ERR
#define SIG_ERR ((sighandler_t) -1)
#endif

/* wrapper to hide signal interface differrencies */
int joe_set_signal(int signum, sighandler_t handler)
{
	int retval;
#ifdef HAVE_SIGACTION
	struct sigaction sact;

	mset(&sact, 0, sizeof(sact));
	sact.sa_handler = handler;
#ifdef SA_INTERRUPT
	sact.sa_flags = SA_INTERRUPT;
#endif
	retval = sigaction(signum, &sact, NULL);
#elif defined(HAVE_SIGVEC)
	struct sigvec svec;

	mset(&svec, 0, sizeof(svec));
	svec.sv_handler = handler;
#ifdef HAVE_SV_INTERRUPT
	svec.sv_flags = SV_INTERRUPT;
#endif
	retval = sigvec(signum, &svec, NULL);
#else
	retval = (signal(signum, handler) != SIG_ERR) ? 0 : -1;
#ifdef HAVE_SIGINTERRUPT
	siginterrupt(signum, 1);
#endif
#endif
	return(retval);
}

/* Helpful little parsing utilities */

/* Skip whitespace and return first non-whitespace character */

int
parse_ws(unsigned char **pp, int cmt)
{
	unsigned char *p = *pp;
	while (*p==' ' || *p=='\t')
		++p;
	if (*p=='\r' || *p=='\n' || *p==cmt)
		*p = 0;
	*pp = p;
	return *p;
}

/* Parse an identifier into a buffer.  Identifier is truncated to a maximum of len chars. */

int
parse_ident(unsigned char **pp, unsigned char *buf, int len)
{
	unsigned char *p = *pp;
	if (joe_isalphx(locale_map,*p)) {
		while(len && joe_isalnux(locale_map,*p))
			*buf++= *p++, --len;
		*buf=0;
		while(joe_isalnux(locale_map,*p))
			++p;
		*pp = p;
		return 0;
	} else
		return -1;
}

/* Parse to next whitespace */

int
parse_tows(unsigned char **pp, unsigned char *buf)
{
	unsigned char *p = *pp;
	while (*p && *p!=' ' && *p!='\t' && *p!='\n' && *p!='\r' && *p!='#')
		*buf++ = *p++;

	*pp = p;
	*buf = 0;
	return 0;
}

/* Parse a keyword */

int
parse_kw(unsigned char **pp, const unsigned char *kw)
{
	unsigned char *p = *pp;
	while(*kw && *kw==*p)
		++kw, ++p;
	if(!*kw && !joe_isalnux(locale_map,*p)) {
		*pp = p;
		return 0;
	} else
		return -1;
}

/* Parse a field */

int
parse_field(unsigned char **pp, const unsigned char *kw)
{
	unsigned char *p = *pp;
	while(*kw && *kw==*p)
		++kw, ++p;
	if(!*kw && (!*p || *p==' ' || *p=='\t' || *p=='#' || *p=='\n' || *p=='\r')) {
		*pp = p;
		return 0;
	} else
		return -1;
}

/* Parse a character */

int
parse_char(unsigned char **pp, unsigned char c)
{
	unsigned char *p = *pp;
	if (*p == c) {
		*pp = p+1;
		return 0;
	} else
		return -1;
}

/*
 * Parse a string into a buffer.  Returns 0 for success.
 * Leaves escape sequences in string.
 */
int
parse_string(unsigned char **pp, unsigned char *buf, int len)
{
	unsigned char *p = *pp;
	if(*p=='\"') {
		++p;
		while(len && *p && *p!='\"')
			if(*p=='\\' && p[1] && len>2) {
				*buf++ = *p++;
				*buf++ = *p++;
				len-=2;
			} else {
				*buf++ = *p++;
				--len;
			}
		*buf = 0;
		while(*p && *p!='\"')
			if(*p=='\\' && p[1])
				p+=2;
			else
				p++;
		if(*p=='\"') {
			*pp= p+1;
			return 0;
		}
	}
	return -1;
}

/* Parse a character range: a-z */
int
parse_range(unsigned char **pp, int *first, int *second)
{
	unsigned char *p = *pp;
	int a, b;
	if(!*p)
		return -1;
	if(*p=='\\' && p[1]) {
		++p;
		if(*p=='n')
			a = '\n';
		else if(*p=='t')
			a = '\t';
		else
			a = *p;
		++p;
	} else
		a = *p++;
	if(*p=='-' && p[1]) {
		++p;
		if(*p=='\\' && p[1]) {
			++p;
			if(*p=='n')
				b = '\n';
			else if(*p=='t')
				b = '\t';
			else
				b = *p;
			++p;
		} else
			b = *p++;
	} else
		b = a;
	*first = a;
	*second = b;
	*pp = p;
	return 0;
}
