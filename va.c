/*
 *	Variable length array of strings
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#include "config.h"
#include "types.h"

__RCSID("$MirOS: contrib/code/jupp/va.c,v 1.10 2018/01/07 20:39:33 tg Exp $");

#include <stdlib.h>

#include "utils.h"
#include "va.h"

aELEMENT *
vamk(int len)
{
	aELEMENT *rv;

	rv = jalloc(NULL, len, sizeof(aELEMENT));
	rv[0] = aterm;
	return (rv);
}

void
varm(aELEMENT *vary)
{
	if (vary) {
		vazap(vary, 0, aLen(vary));
		jfree(vary);
	}
}

int alen(aELEMENT *ary)
{
	aELEMENT *beg = ary;

	if (!ary)
		return (0);

	while (acmp(*ary, aterm))
		++ary;
	return (ary - beg);
}

aELEMENT *
vaensure(aELEMENT *vary, int len)
{
	aELEMENT *rv;

	if (vary && len > aSiz(vary))
		len += (len >> 2);
	rv = jalloc(vary, len, sizeof(aELEMENT));
	if (!vary)
		rv[0] = aterm;
	return (rv);
}

aELEMENT *vazap(aELEMENT *vary, int pos, int n)
{
	if (vary) {
		int x;

		if (pos < aLen(vary)) {
			if (pos + n <= aLen(vary)) {
				for (x = pos; x != pos + n; ++x)
					adel(vary[x]);
			} else {
				for (x = pos; x != aLen(vary); ++x)
					adel(vary[x]);
			}
		}
	}
	return vary;
}

aELEMENT *vatrunc(aELEMENT *vary, int len)
{
	if (!vary || len > aLEN(vary))
		vary = vaensure(vary, len);
	if (len < aLen(vary)) {
		vary = vazap(vary, len, aLen(vary) - len);
		vary[len] = vary[aLen(vary)];
		aLen(vary) = len;
	} else if (len > aLen(vary)) {
		vary = vafill(vary, aLen(vary), ablank, len - aLen(vary));
	}
	return vary;
}

aELEMENT *vafill(aELEMENT *vary, int pos, aELEMENT el, int len)
{
	int olen = aLEN(vary), x;

	if (!vary || pos + len > aSIZ(vary))
		vary = vaensure(vary, pos + len);
	if (pos + len > olen) {
		vary[pos + len] = vary[olen];
		aLen(vary) = pos + len;
	}
	for (x = pos; x != pos + len; ++x)
		vary[x] = adup(el);
	if (pos > olen)
		vary = vafill(vary, pos, ablank, pos - olen);
	return vary;
}

aELEMENT *vandup(aELEMENT *vary, int pos, aELEMENT *array, int len)
{
	int olen = aLEN(vary), x;

	if (!vary || pos + len > aSIZ(vary))
		vary = vaensure(vary, pos + len);
	if (pos + len > olen) {
		vary[pos + len] = vary[olen];
		aLen(vary) = pos + len;
	}
	if (pos > olen)
		vary = vafill(vary, olen, ablank, pos - olen);
	for (x = 0; x != len; ++x)
		vary[x + pos] = adup(array[x]);
	return vary;
}

aELEMENT *vadup(aELEMENT *vary)
{
	return vandup(NULL, 0, vary, aLEN(vary));
}

aELEMENT *_vaset(aELEMENT *vary, int pos, aELEMENT el)
{
	if (!vary || pos + 1 > aSIZ(vary))
		vary = vaensure(vary, pos + 1);
	if (pos > aLen(vary)) {
		vary = vafill(vary, aLen(vary), ablank, pos - aLen(vary));
		vary[pos + 1] = vary[pos];
		vary[pos] = el;
		aLen(vary) = pos + 1;
	} else if (pos == aLen(vary)) {
		vary[pos + 1] = vary[pos];
		vary[pos] = el;
		aLen(vary) = pos + 1;
	} else {
		adel(vary[pos]);
		vary[pos] = el;
	}
	return vary;
}

static int _acmp(aELEMENT *a, aELEMENT *b)
{
	return acmp(*a, *b);
}

aELEMENT *vasort(aELEMENT *ary, int len)
{
	if (ary && len)
		qsort(ary, len, sizeof(aELEMENT),
		    (int (*)(const void *, const void *))_acmp);
	return ary;
}

aELEMENT *
vawords(aELEMENT *a, unsigned char *s, int len,
    const unsigned char *sep, int seplen)
{
	int x;

	if (!a)
		a = vamk(10);
	else
		a = vatrunc(a, 0);
 loop:
	x = vsspan(s, len, sep, seplen);
	s += x;
	len -= x;
	if (len) {
		x = vsscan(s, len, sep, seplen);
		if (x != ~0) {
			a = vaadd(a, vsncpy(vsmk(x), 0, s, x));
			s += x;
			len -= x;
			if (len)
				goto loop;
		} else
			a = vaadd(a, vsncpy(vsmk(len), 0, s, len));
	}
	return a;
}
