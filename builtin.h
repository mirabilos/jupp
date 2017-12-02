#ifdef EXTERN_RC_C
__IDSTRING(rcsid_builtin_h, "$MirOS: contrib/code/jupp/builtin.h,v 1.4 2017/12/02 17:00:48 tg Exp $");
#endif

/* Support for built-in config files */

typedef struct jfile {
	FILE *f;		/* Regular file, or NULL for built-in */
	unsigned char *p;	/* Built-in file pointer */
} JFILE;

JFILE *jfopen(unsigned char *name, const char *mode);
unsigned char *jfgets(unsigned char *buf,int len,JFILE *f);
int jfclose(JFILE *f);

extern unsigned char *builtins[];
