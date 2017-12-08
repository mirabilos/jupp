#ifndef _JOE_CONFIG_H
#define _JOE_CONFIG_H

/* see bottom for RCSID on this one */

#ifndef TEST
#include "autoconf.h"
#else
#define HAVE_CTIME 1
#define HAVE_STRLCAT 1
#define HAVE_STRLCPY 1
#define HAVE_DECL_STRLCAT 1
#define HAVE_DECL_STRLCPY 1
#define HAVE_GETCWD 1
#define HAVE_MKSTEMP 1
#define HAVE_SNPRINTF 1
#define HAVE_SYS_TYPES_H 1
#define HAVE_SYS_STAT_H 1
#define HAVE_DIRENT_H 1
#define HAVE_LIMITS_H 1
#define HAVE_SIGNAL_H 1
#define HAVE_STDLIB_H 1
#define HAVE_UNISTD_H 1
#define RETSIGTYPE void
#endif

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_SNPRINTF

#define joe_snprintf_0(buf,len,fmt) snprintf((buf),(len),(fmt))
#define joe_snprintf_1(buf,len,fmt,a) snprintf((buf),(len),(fmt),(a))
#define joe_snprintf_2(buf,len,fmt,a,b) snprintf((buf),(len),(fmt),(a),(b))
#define joe_snprintf_3(buf,len,fmt,a,b,c) snprintf((buf),(len),(fmt),(a),(b),(c))
#define joe_snprintf_4(buf,len,fmt,a,b,c,d) snprintf((buf),(len),(fmt),(a),(b),(c),(d))
#define joe_snprintf_5(buf,len,fmt,a,b,c,d,e) snprintf((buf),(len),(fmt),(a),(b),(c),(d),(e))
#define joe_snprintf_6(buf,len,fmt,a,b,c,d,e,f) snprintf((buf),(len),(fmt),(a),(b),(c),(d),(e),(f))
#define joe_snprintf_7(buf,len,fmt,a,b,c,d,e,f,g) snprintf((buf),(len),(fmt),(a),(b),(c),(d),(e),(f),(g))
#define joe_snprintf_8(buf,len,fmt,a,b,c,d,e,f,g,h) snprintf((buf),(len),(fmt),(a),(b),(c),(d),(e),(f),(g),(h))
#define joe_snprintf_9(buf,len,fmt,a,b,c,d,e,f,g,h,i) snprintf((buf),(len),(fmt),(a),(b),(c),(d),(e),(f),(g),(h),(i))
#define joe_snprintf_10(buf,len,fmt,a,b,c,d,e,f,g,h,i,j) snprintf((buf),(len),(fmt),(a),(b),(c),(d),(e),(f),(g),(h),(i),(j))

#else

#define joe_snprintf_0(buf,len,fmt) sprintf((buf),(fmt))
#define joe_snprintf_1(buf,len,fmt,a) sprintf((buf),(fmt),(a))
#define joe_snprintf_2(buf,len,fmt,a,b) sprintf((buf),(fmt),(a),(b))
#define joe_snprintf_3(buf,len,fmt,a,b,c) sprintf((buf),(fmt),(a),(b),(c))
#define joe_snprintf_4(buf,len,fmt,a,b,c,d) sprintf((buf),(fmt),(a),(b),(c),(d))
#define joe_snprintf_5(buf,len,fmt,a,b,c,d,e) sprintf((buf),(fmt),(a),(b),(c),(d),(e))
#define joe_snprintf_6(buf,len,fmt,a,b,c,d,e,f) sprintf((buf),(fmt),(a),(b),(c),(d),(e),(f))
#define joe_snprintf_7(buf,len,fmt,a,b,c,d,e,f,g) sprintf((buf),(fmt),(a),(b),(c),(d),(e),(f),(g))
#define joe_snprintf_8(buf,len,fmt,a,b,c,d,e,f,g,h) sprintf((buf),(fmt),(a),(b),(c),(d),(e),(f),(g),(h))
#define joe_snprintf_9(buf,len,fmt,a,b,c,d,e,f,g,h,i) sprintf((buf),(fmt),(a),(b),(c),(d),(e),(f),(g),(h),(i))
#define joe_snprintf_10(buf,len,fmt,a,b,c,d,e,f,g,h,i,j) sprintf((buf),(fmt),(a),(b),(c),(d),(e),(f),(g),(h),(i),(j))

#endif

#include <stdio.h>
#ifndef EOF
#define EOF (-1)
#endif
#define NO_MORE_DATA EOF

#ifdef PAGE_SIZE
#define PGSIZE PAGE_SIZE
#else
#define PGSIZE 4096
#endif
#define SEGSIZ PGSIZE
#define LPGSIZE 12
#define ILIMIT (PGSIZE*1024)
#define HTSIZE 2048

#if !defined(__GNUC__) && !defined(__attribute__)
#define __attribute__(p) /* nothing */
#endif

#ifdef HAVE_GCC_ATTRIBUTE_BOUNDED
#define ATTR_BOUNDED(p)	__attribute__((__bounded__ p))
#else
#define ATTR_BOUNDED(p)	/* nothing */
#endif

#if !HAVE_DECL_STRLCAT
size_t strlcat(char *, const char *, size_t)
    ATTR_BOUNDED((__string__, 1, 3));
#endif
#if !HAVE_DECL_STRLCPY
size_t strlcpy(char *, const char *, size_t)
    ATTR_BOUNDED((__string__, 1, 3));
#endif

/* from mksh */

#define BIT(i)		(1U << (i))
#define NELEM(a)	(sizeof(a) / sizeof((a)[0]))

#if defined(MirBSD) && (MirBSD >= 0x09A1) && \
    defined(__ELF__) && defined(__GNUC__) && \
    !defined(__llvm__) && !defined(__NWCC__)
/*
 * We got usable __IDSTRING __COPYRIGHT __RCSID __SCCSID macros
 * which work for all cases; no need to redefine them using the
 * "portable" macros from below when we might have the "better"
 * gcc+ELF specific macros or other system dependent ones.
 */
#else
#undef __IDSTRING
#undef __IDSTRING_CONCAT
#undef __IDSTRING_EXPAND
#undef __COPYRIGHT
#undef __RCSID
#undef __SCCSID
#define __IDSTRING_CONCAT(l,p)		__LINTED__ ## l ## _ ## p
#define __IDSTRING_EXPAND(l,p)		__IDSTRING_CONCAT(l,p)
#ifdef MKSH_DONT_EMIT_IDSTRING
#define __IDSTRING(prefix, string)	/* nothing */
#elif defined(__ELF__) && defined(__GNUC__) && \
    !defined(__llvm__) && !defined(__NWCC__) && !defined(NO_ASM)
#define __IDSTRING(prefix, string)				\
	__asm__(".section .comment"				\
	"\n	.ascii	\"@(\"\"#)" #prefix ": \""		\
	"\n	.asciz	\"" string "\""				\
	"\n	.previous")
#else
#define __IDSTRING(prefix, string)				\
	static const char __IDSTRING_EXPAND(__LINE__,prefix) []	\
	    __attribute__((__used__)) = "@(""#)" #prefix ": " string
#endif
#define __COPYRIGHT(x)		__IDSTRING(copyright,x)
#define __RCSID(x)		__IDSTRING(rcsid,x)
#define __SCCSID(x)		__IDSTRING(sccsid,x)
#endif

#ifdef EXTERN
__IDSTRING(rcsid_config_h, "$MirOS: contrib/code/jupp/config.h,v 1.14 2017/12/08 02:17:21 tg Exp $");
#endif

#endif /* ifndef _JOE_CONFIG_H */
