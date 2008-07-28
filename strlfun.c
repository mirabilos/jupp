/*-
 * Copyright (c) 2006, 2008
 *	Thorsten Glaser <tg@mirbsd.org>
 *
 * Provided that these terms and disclaimer and all copyright notices
 * are retained or reproduced in an accompanying document, permission
 * is granted to deal in this work without restriction, including un-
 * limited rights to use, publicly perform, distribute, sell, modify,
 * merge, give away, or sublicence.
 *
 * This work is provided "AS IS" and WITHOUT WARRANTY of any kind, to
 * the utmost extent permitted by applicable law, neither express nor
 * implied; without malicious intent or gross negligence. In no event
 * may a licensor, author or contributor be held liable for indirect,
 * direct, other damage, loss, or other issues arising in any way out
 * of dealing in the work, even if advised of the possibility of such
 * damage or existence of a defect, except proven that it results out
 * of said person's immediate fault when using the work as intended.
 *-
 * The original implementations of strlcpy(3) and strlcat(3) are from
 * Todd C. Miller; the licence is reproduced below. However, this ap-
 * plies only to the strlcpy(3) portion of the code, as Thorsten Gla-
 * ser write the following strlcat(3) implementation according to the
 * spec. Both functions below have been optimised according to sugge-
 * stions from Bodo Eggert. Thorsten Glaser also has merged this code
 * with strxfrm(3) for ISO-10646-only systems and wrote the wide char
 * variants wcslcat(3), wcslcpy(3), and wcsxfrm(3) (see wcslfun.c).
 */

#ifdef STRXFRM
#undef HAVE_STRLCPY
#undef HAVE_STRLCAT
#define HAVE_STRLCPY	0
#define HAVE_STRLCAT	1
#define strlcpy		strxfrm
#endif

#include <sys/types.h>
#if defined(_KERNEL) || defined(_STANDALONE)
#include <lib/libkern/libkern.h>
#undef HAVE_STRLCPY
#undef HAVE_STRLCAT
#else
#include <stddef.h>	/* for size_t in user space (SUSv3) */
#if defined(HAVE_CONFIG_H) && (HAVE_CONFIG_H != 0)
/* usually when packaged with third-party software */
#ifdef CONFIG_H_FILENAME
#include CONFIG_H_FILENAME
#else
#include "config.h"
#endif
#endif
/* do not include <string.h> to prevent redefinition warnings */
extern size_t strlen(const char *);
#endif

#ifndef __IDSTRING
#undef __RCSID
#undef __IDSTRING
#undef __IDSTRING_CONCAT
#undef __IDSTRING_EXPAND
#define __IDSTRING_CONCAT(l,p)		__LINTED__ ## l ## _ ## p
#define __IDSTRING_EXPAND(l,p)		__IDSTRING_CONCAT(l,p)
#define __IDSTRING(prefix, string)				\
	static const char __IDSTRING_EXPAND(__LINE__,prefix) []	\
	    __attribute__((used)) = "@(""#)" #prefix ": " string
#define __RCSID(x)			__IDSTRING(rcsid,x)
#endif

#ifndef __predict_true
#define __predict_true(exp)	((exp) != 0)
#endif
#ifndef __predict_false
#define __predict_false(exp)	((exp) != 0)
#endif

#if !defined(_KERNEL) && !defined(_STANDALONE)
__RCSID("$MirOS: contrib/code/jupp/strlfun.c,v 1.7 2008/07/28 00:03:25 tg Exp $");
__RCSID("$miros: src/lib/libc/string/strlfun.c,v 1.16 2008/07/07 12:59:51 tg Stab $");
#endif

/* (multibyte) string functions */
#undef NUL
#undef char_t
#define NUL		'\0'
#define char_t		char

size_t strlcat(char_t *, const char_t *, size_t);
size_t strlcpy(char_t *, const char_t *, size_t);

#if !defined(HAVE_STRLCAT) || (HAVE_STRLCAT == 0)
/*
 * Appends src to string dst of size dlen (unlike strncat, dlen is the
 * full size of dst, not space left).  At most dlen-1 characters
 * will be copied.  Always NUL terminates (unless dlen <= strlen(dst)).
 * Returns strlen(src) + MIN(dlen, strlen(initial dst)), without the
 * trailing NUL byte counted.  If retval >= dlen, truncation occurred.
 */
size_t
strlcat(char_t *dst, const char_t *src, size_t dlen)
{
	size_t n = 0, slen;

	slen = strlen(src);
	while (__predict_true(n + 1 < dlen && dst[n] != NUL))
		++n;
	if (__predict_false(dlen == 0 || dst[n] != NUL))
		return (dlen + slen);
	while (__predict_true((slen > 0) && (n < (dlen - 1)))) {
		dst[n++] = *src++;
		--slen;
	}
	dst[n] = NUL;
	return (n + slen);
}
#endif /* !HAVE_STRLCAT */

#if !defined(HAVE_STRLCPY) || (HAVE_STRLCPY == 0)
/* $OpenBSD: strlcpy.c,v 1.11 2006/05/05 15:27:38 millert Exp $ */

/*-
 * Copyright (c) 1998 Todd C. Miller <Todd.Miller@courtesan.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * Copy src to string dst of size siz.  At most siz-1 characters
 * will be copied.  Always NUL terminates (unless siz == 0).
 * Returns strlen(src); if retval >= siz, truncation occurred.
 */
size_t
strlcpy(char_t *dst, const char_t *src, size_t siz)
{
	const char_t *s = src;

	if (__predict_false(siz == 0))
		goto traverse_src;

	/* copy as many chars as will fit */
	while (--siz && (*dst++ = *s++))
		;

	/* not enough room in dst */
	if (__predict_false(siz == 0)) {
		/* safe to NUL-terminate dst since we copied <= siz-1 chars */
		*dst = NUL;
 traverse_src:
		/* traverse rest of src */
		while (*s++)
			;
	}

	/* count does not include NUL */
	return (s - src - 1);
}
#endif /* !HAVE_STRLCPY */
