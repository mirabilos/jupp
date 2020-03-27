/*
 *	Character sets
 *	Copyright
 *		(C) 2004 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */

#ifndef _Icharmap
#define _Icharmap 1

#ifdef EXTERN
__RCSID("$MirOS: contrib/code/jupp/charmap.h,v 1.13 2020/03/27 06:08:12 tg Exp $");
#endif

/* For sorted from_map entries */

struct pair {
	int first;			/* Unicode */
	int last;			/* Byte */
};

/* A character set */

union charmap;

struct charmap_head {
	/* linked list of loaded character maps */
	union charmap *next;
	/* name of this character map, NULL = JOE_MAPUTFCS (UTF-8) */
	const unsigned char *cs;
};

struct charmap_byte {
	struct charmap_head head;

	/* convert byte to UCS */
	const int *to_map;

	/* number of pairs in from_map */
	int from_size;

	/* case conversion */
	unsigned char lower_map[256];
	unsigned char upper_map[256];

	/* bitmap of attributes */
	unsigned char alnux_map[32];
	unsigned char alphx_map[32];
	unsigned char print_map[32];

	/* convert UCS to bytes */
	struct pair from_map[256 + 2];
};

union charmap {
	struct charmap_head head;
	struct charmap_byte byte;
};

/* Predicates */

extern const unsigned char JOE_MAPUTFCS[];
#define joe_maputf(map)		((map)->head.cs == NULL)
#define joe_mapname(map)	(joe_maputf(map) ? JOE_MAPUTFCS : \
				    (map)->head.cs)

#define joe_ispunct(map,c)	(joe_maputf(map) ? joe_iswpunct(c) : \
				    byte_ispunct((map),(c)))
#define joe_isprint(map,c)	(joe_maputf(map) ? joe_iswprint(c) : \
				    byte_isprint((map),(c)))
#define joe_isspace(map,c)	(joe_maputf(map) ? joe_iswspace(c) : \
				    ((c) == 32 || ((c) >= 9 && (c) <= 13)))
#define joe_isalphx(map,c)	(joe_maputf(map) ? joe_iswalpha(c) : \
				    byte_isalphx((map),(c)))
#define joe_isalnux(map,c)	(joe_maputf(map) ? joe_iswalnum(c) : \
				    byte_isalnux((map),(c)))
int joe_isblank(int c);
int joe_isspace_eof(union charmap *map, int c);

/* Conversion functions */

#define joe_tolower(map,c)	(joe_maputf(map) ? joe_towlower(c) : \
				    byte_tolower((map),(c)))
#define joe_toupper(map,c)	(joe_maputf(map) ? joe_towupper(c) : \
				    byte_toupper((map),(c)))
#define joe_to_uni(map,c)	(joe_maputf(map) ? (c) : \
				    (map)->byte.to_map[(c)])
#define joe_from_uni(map,c)	(joe_maputf(map) ? (c) : \
				    byte_from_uni((map),(c)))
unsigned char *joe_strtolower(unsigned char *s);

/* Find (load if necessary) a character set */
union charmap *find_charmap(const unsigned char *name);

/* Get available encodings */
unsigned char **get_encodings(void);

int byte_from_uni(union charmap *, int);
int byte_ispunct(union charmap *, int);
int byte_isprint(union charmap *, int);
int byte_isalphx(union charmap *, int);
int byte_isalnux(union charmap *, int);
int byte_tolower(union charmap *, int);
int byte_toupper(union charmap *, int);

#include "utf8.h"

#endif
