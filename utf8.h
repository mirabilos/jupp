/*
 *	UTF-8 Utilities
 *	Copyright
 *		(C) 2004 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#ifndef _Iutf8
#define _Iutf8 1

#ifdef EXTERN
__IDSTRING(rcsid_utf8_h, "$MirOS: contrib/code/jupp/utf8.h,v 1.10 2018/08/10 02:53:45 tg Exp $");
#endif

#include "i18n.h"

/* UTF-8 Encoder
 *
 * c is a UCS character.
 * buf is 7 byte buffer- utf-8 coded character is written to this followed by a 0 termination.
 * returns length (not including terminator).
 */

int utf8_encode(unsigned char *buf,int c);

/* UTF-8 decoder state machine */

struct utf8_sm {
	unsigned char buf[8];	/* Record of sequence */
	int ptr;		/* Record pointer */
	int state;		/* Current state.  0 = idle, anything else is no. of chars left in sequence */
	int accu;		/* Character accumulator */
	int minv;		/* Minimum value, for decoder */
};

/* UTF-8 Decoder
 *
 * Returns 0 - 7FFFFFFF: decoded character
 *                   -1: character accepted, nothing decoded yet.
 *                   -2: incomplete sequence
 *                   -3: no sequence started, but character is between 128 - 191, 254 or 255
 */

int utf8_decode(struct utf8_sm *utf8_sm,unsigned char c);

int utf8_decode_string(unsigned char *s);

int utf8_decode_fwrd(unsigned char **p,int *plen);

/* Initialize state machine */

void utf8_init(struct utf8_sm *utf8_sm);

void joe_locale(void);
void to_utf8(struct charmap *map,unsigned char *s,int c);
int from_utf8(struct charmap *map,unsigned char *s);

extern int utf8;

int mk_wcwidth(int wide,int c);

extern struct charmap *locale_map;	/* Default bytemap of terminal */
extern struct charmap *utf8_map;	/* Bytemap for UTF-8 */

#endif
