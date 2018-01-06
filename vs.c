/*
 *	Variable length strings
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#include "config.h"
#include "types.h"

__RCSID("$MirOS: contrib/code/jupp/vs.c,v 1.11 2017/12/08 02:00:43 tg Exp $");

#include <stdlib.h>

#include "blocks.h"
#include "utils.h"
#include "vs.h"

sELEMENT *vsmk(int len)
{
	int *new = malloc((1 + len) * sizeof(sELEMENT) + 2 * sizeof(int));

	new[0] = len;
	new[1] = 0;
	((sELEMENT *)(new + 2))[0] = sdup(sterm);
	return (sELEMENT *)(new + 2);
}

void vsrm(sELEMENT *vary)
{
	if (vary)
		free((int *)vary - 2);
}

int slen(const sELEMENT *ary)
{
	if (ary) {
		const sELEMENT *beg = ary;
		while (scmp(*ary, sterm))
			++ary;
		return ary - beg;
	} else
		return 0;
}

sELEMENT *vsensure(sELEMENT *vary, int len)
{
	if (!vary)
		vary = vsmk(len);
	else if (len > sSiz(vary)) {
		len += (len >> 2);
		vary = (sELEMENT *)(2 + (int *)realloc((int *)vary - 2, (len + 1) * sizeof(sELEMENT) + 2 * sizeof(int)));

		sSiz(vary) = len;
	}
	return vary;
}

sELEMENT *vstrunc(sELEMENT *vary, int len)
{
	if (!vary || len > sLEN(vary))
		vary = vsensure(vary, len + 16);
	if (len < sLen(vary)) {
		vary[len] = vary[sLen(vary)];
		sLen(vary) = len;
	} else if (len > sLen(vary)) {
		vary = vsfill(vary, sLen(vary), sblank, len - sLen(vary));
	}
	return vary;
}

sELEMENT *vsfill(sELEMENT *vary, int pos, sELEMENT el, int len)
{
	int olen = sLEN(vary), x;

	if (!vary || pos + len > sSIZ(vary))
		vary = vsensure(vary, pos + len);
	if (pos + len > olen) {
		vary[pos + len] = vary[olen];
		sLen(vary) = pos + len;
	}
	for (x = pos; x != pos + len; ++x)
		vary[x] = sdup(el);
	if (pos > olen)
		vary = vsfill(vary, pos, sblank, pos - olen);
	return vary;
}

sELEMENT *vsncpy(sELEMENT *vary, int pos, const sELEMENT *array, int len)
{
	int olen = sLEN(vary);

	if (!vary || pos + len > sSIZ(vary))
		vary = vsensure(vary, pos + len);
	if (pos + len > olen) {
		vary[pos + len] = vary[olen];
		sLen(vary) = pos + len;
	}
	if (pos > olen)
		vary = vsfill(vary, olen, sblank, pos - olen);
#ifdef TEST
	memmove(vary + pos, array, len * sizeof(sELEMENT));
#else
	mmove(vary + pos, array, len * sizeof(sELEMENT));
#endif
	return vary;
}

sELEMENT *vsndup(sELEMENT *vary, int pos, sELEMENT *array, int len)
{
	int olen = sLEN(vary), x;

	if (!vary || pos + len > sSIZ(vary))
		vary = vsensure(vary, pos + len);
	if (pos + len > olen) {
		vary[pos + len] = vary[olen];
		sLen(vary) = pos + len;
	}
	if (pos > olen)
		vary = vsfill(vary, olen, sblank, pos - olen);
	for (x = pos; x != len; ++x)
		vary[x] = sdup(array[x]);
	return vary;
}

sELEMENT *vsdup(sELEMENT *vary)
{
	return vsndup(NULL, 0, vary, sLEN(vary));
}

sELEMENT *_vsset(sELEMENT *vary, int pos, sELEMENT el)
{
	if (!vary || pos + 1 > sSIZ(vary))
		vary = vsensure(vary, pos + 1);
	if (pos > sLen(vary)) {
		vary = vsfill(vary, sLen(vary), sblank, pos - sLen(vary));
		vary[pos + 1] = vary[pos];
		vary[pos] = el;
		sLen(vary) = pos + 1;
	} else if (pos == sLen(vary)) {
		vary[pos + 1] = vary[pos];
		vary[pos] = el;
		sLen(vary) = pos + 1;
	} else {
		sdel(vary[pos]);
		vary[pos] = el;
	}
	return vary;
}

int vsbsearch(const sELEMENT *ary, int len, sELEMENT el)
{
	int x, y, z;

	if (!ary || !len)
		return 0;
	y = len;
	x = 0;
	z = ~0;
	while (z != (x + y) / 2) {
		z = (x + y) / 2;
		switch (scmp(el, ary[z])) {
		case 1:
			x = z;
			break;
		case -1:
			y = z;
			break;
		case 0:
			return z;
		}
	}
	return y;
}

int vscmpn(sELEMENT *a, int alen, sELEMENT *b, int blen)
{
	int x, l;
	int t;

	if (!a && !b)
		return 0;
	if (!a)
		return -1;
	if (!b)
		return 1;
	if (alen > blen)
		l = sLen(a);
	else
		l = blen;
	for (x = 0; x != l; ++x)
		if ((t = scmp(a[x], b[x])) != 0)
			return t;
	if (alen > blen)
		return 1;
	if (alen < blen)
		return -1;
	return 0;
}

int vscmp(sELEMENT *a, sELEMENT *b)
{
	return vscmpn(sv(a), sv(b));
}

int vsscan(const sELEMENT *a, int alen, const sELEMENT *b, int blen)
{
	int x;

	for (x = 0; x != alen; ++x) {
		int z = vsbsearch(b, blen, a[x]);

		if (z < blen && !scmp(b[z], a[x]))
			return x;
	}
	return ~0;
}

int
vsspan(const sELEMENT *a, int alen, const sELEMENT *b, int blen)
{
	int x;

	/* should not happen */
	if (!b)
		return (0);

	for (x = 0; x != alen; ++x) {
		int z = vsbsearch(b, blen, a[x]);

		if (z == blen || scmp(b[z], a[x]))
			break;
	}
	return x;
}
