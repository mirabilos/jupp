/*
 *	Various utilities
 *
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *		(C) 2001 Marek 'Marx' Grac
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#ifndef _JOE_UTILS_H
#define _JOE_UTILS_H 1

#ifdef EXTERN_B_C
__IDSTRING(rcsid_utils_h, "$MirOS: contrib/code/jupp/utils.h,v 1.14 2018/11/11 18:20:52 tg Exp $");
#endif

#include <signal.h>

/*
 * Functions which return minimum/maximum of two numbers
 */
unsigned int uns_min(unsigned int a, unsigned int b);
signed int int_min(signed int a, int signed b);
signed long long_max(signed long a, signed long b);
signed long long_min(signed long a, signed long b);

/* Versions of 'read' and 'write' which automatically retry when interrupted */
ssize_t joe_read(int fd, void *buf, size_t siz);
ssize_t joe_write(int fd, const void *buf, size_t siz);

/* Similarily, read and write an exact amount (up to EOF) */
ssize_t joe_readex(int, void *, size_t);
ssize_t joe_writex(int, const void *, size_t);
/* these return -2 if the error occurs after bytes had been processed */

#ifndef HAVE_SIGHANDLER_T
typedef RETSIGTYPE (*sighandler_t)(int);
#endif

#ifdef NEED_TO_REINSTALL_SIGNAL
#define REINSTALL_SIGHANDLER(sig, handler) joe_set_signal(sig, handler)
#else
#define REINSTALL_SIGHANDLER(sig, handler) do {} while(0)
#endif

/* wrapper to hide signal interface differrencies */
int joe_set_signal(int signum, sighandler_t handler);

int parse_ws(unsigned char **p, int cmt);
int parse_ident(unsigned char **p, unsigned char *buf, int len);
int parse_tows(unsigned char **p, unsigned char *buf);
int parse_kw(unsigned char **p, const unsigned char *kw);
int parse_field(unsigned char **p, const unsigned char *field);
int parse_char(unsigned char **p, unsigned char c);
int parse_string(unsigned char **p, unsigned char *buf, int len);
int parse_range(unsigned char **p, int *first, int *second);

void tty_xonoffbaudrst(void);

/* from compat.c */

#define USTOL_AUTO	0x00
#define USTOL_DEC	0x01
#define USTOL_HEX	0x02
#define USTOL_OCT	0x03
#define USTOL_LTRIM	0x04
#define USTOL_RTRIM	0x08
#define USTOL_TRIM	0x0C
#define USTOL_EOS	0x10

long ustol(void *, void **, int);
long ustolb(void *, void **, long, long, int);

/* arbitrary, but at least 4 */
#define USTOC_MAX	4

size_t ustoc_hex(const void *, int *, size_t);
size_t ustoc_oct(const void *, int *, size_t);

/* from selinux.c */

int copy_security_context(const char *, const char *);

#endif
