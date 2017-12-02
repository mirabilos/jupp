#ifndef _Ii18n
#define _Ii18n 1

#ifdef EXTERN
__RCSID("$MirOS: contrib/code/jupp/i18n.h,v 1.9 2017/12/02 02:07:28 tg Exp $");
#endif

#ifdef TEST_I18N
int joe_iswupper PARAMS((struct charmap *,int c));
int joe_iswlower PARAMS((struct charmap *,int c));
#endif

/* the following two include _ */
int joe_iswalpha PARAMS((struct charmap *,int c));
int joe_iswalnum PARAMS((struct charmap *,int c));

int joe_iswdigit PARAMS((struct charmap *,int c));
int joe_iswspace PARAMS((struct charmap *,int c));
#ifdef TEST_I18N
int joe_iswcntrl PARAMS((struct charmap *,int c));
#endif
int joe_iswpunct PARAMS((struct charmap *,int c));
#ifdef TEST_I18N
int joe_iswgraph PARAMS((struct charmap *,int c));
#endif
int joe_iswprint PARAMS((struct charmap *,int c));
#ifdef TEST_I18N
int joe_iswxdigit PARAMS((struct charmap *,int c));
int joe_iswblank PARAMS((struct charmap *,int c));
#endif

int joe_wcwidth PARAMS((int wide, unsigned int c));
/* Looking for wswidth? Take a look at scrn.c/txtwidth() */

int joe_towupper PARAMS((struct charmap *,int c));
int joe_towlower PARAMS((struct charmap *,int c));

extern unsigned char unictrlbuf[11];
int unictrl PARAMS((unsigned int c));

#endif
