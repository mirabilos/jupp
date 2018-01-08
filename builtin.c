/*
 *	Built-in config files
 *	Copyright
 *		(C) 2006 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */

#include "config.h"

__RCSID("$MirOS: contrib/code/jupp/builtin.c,v 1.10 2018/01/08 00:48:06 tg Exp $");

#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "builtin.h"
#include "utils.h"

#define zcmp(a,b) strcmp((char *)(a), (char *)(b))

JFILE *
jfopen(const unsigned char *name, const char *mode)
{
	if (name[0] == '*') {
		int x;
		char *xname, *cp;

		xname = strdup((const char *)name + 1);
		cp = xname;
		while ((x = *cp++)) {
			if (x >= 'A' && x <= 'Z')
				cp[-1] = x - 'A' + 'a';
		}

		for (x = 0; builtins[x]; x += 2) {
			if (!zcmp(builtins[x], xname)) {
				JFILE *j = malloc(sizeof(JFILE));
				j->f = 0;
				j->p = builtins[x + 1];
				free(xname);
				return j;
			}
		}
		free(xname);
		return 0;
	} else {
		FILE *f = fopen((const char *)name, (const char *)mode);
		if (f) {
			JFILE *j = malloc(sizeof(JFILE));
			j->f = f;
			j->p = 0;
			return j;
		} else {
			return 0;
		}
	}
}

int jfclose(JFILE *f)
{
	int rtn = 0;
	if (f->f)
		rtn = fclose(f->f);
	free(f);
	return rtn;
}

/*XXX fails to honour len (= 1024, in practice) for builtins */
unsigned char *
jfgets(unsigned char *buf, int len, JFILE *f)
{
	if (f->f)
		return (unsigned char *)fgets((char *)buf, len, f->f);
	else {
		if (f->p[0]) {
			int x;
			for (x = 0; f->p[x] && f->p[x] != '\n'; ++x)
				buf[x] = f->p[x];
			if (f->p[x] == '\n') {
				buf[x++] = '\n';
			}
			buf[x] = 0;
			f->p += x;
			return buf;
		} else
			return 0;
	}
}
