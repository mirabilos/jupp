#ifndef JUPP_BUILTIN_H
#define JUPP_BUILTIN_H

#ifdef EXTERN_RC_C
__IDSTRING(rcsid_builtin_h, "$MirOS: contrib/code/jupp/builtin.h,v 1.7 2020/03/27 06:38:55 tg Exp $");
#endif

/* Support for built-in config files */

typedef struct jfile {
	FILE *f;		/* Regular file, or NULL for built-in */
	const unsigned char *p;	/* Built-in file pointer */
} JFILE;

JFILE *jfopen(const unsigned char *name, const char *mode);
unsigned char *jfgets(unsigned char *buf,int len,JFILE *f);
int jfclose(JFILE *f);

extern const unsigned char * const builtins[];

#endif
