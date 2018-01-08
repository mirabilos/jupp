#ifndef _Ii18n
#define _Ii18n 1

#ifdef EXTERN
__IDSTRING(rcsid_i18n_h, "$MirOS: contrib/code/jupp/i18n.h,v 1.12 2018/01/08 02:01:20 tg Exp $");
#endif

#ifdef TEST_I18N
int joe_iswupper(struct charmap *,int c);
int joe_iswlower(struct charmap *,int c);
#endif

/* the following two include _ */
int joe_iswalpha(struct charmap *,int c);
int joe_iswalnum(struct charmap *,int c);

int joe_iswdigit(struct charmap *,int c);
int joe_iswspace(struct charmap *,int c);
#ifdef TEST_I18N
int joe_iswcntrl(struct charmap *,int c);
#endif
int joe_iswpunct(struct charmap *,int c);
#ifdef TEST_I18N
int joe_iswgraph(struct charmap *,int c);
#endif
int joe_iswprint(struct charmap *,int c);
#ifdef TEST_I18N
int joe_iswxdigit(struct charmap *,int c);
int joe_iswblank(struct charmap *,int c);
#endif

int joe_wcwidth(unsigned int c);
/* Looking for wswidth? Take a look at scrn.c/txtwidth() */

int joe_towupper(struct charmap *,int c);
int joe_towlower(struct charmap *,int c);

extern unsigned char unictrlbuf[11];
int unictrl(unsigned int c);

#endif
