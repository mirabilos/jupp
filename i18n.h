#ifndef JUPP_I18N_H
#define JUPP_I18N_H

#ifdef EXTERN
__IDSTRING(rcsid_i18n_h, "$MirOS: contrib/code/jupp/i18n.h,v 1.14 2020/03/27 06:38:56 tg Exp $");
#endif

#ifdef TEST_I18N
int joe_iswupper(int c);
int joe_iswlower(int c);
#endif

/* the following two include _ */
int joe_iswalpha(int c);
int joe_iswalnum(int c);

int joe_iswdigit(int c);
int joe_iswspace(int c);
#ifdef TEST_I18N
int joe_iswcntrl(int c);
#endif
int joe_iswpunct(int c);
#ifdef TEST_I18N
int joe_iswgraph(int c);
#endif
int joe_iswprint(int c);
#ifdef TEST_I18N
int joe_iswxdigit(int c);
int joe_iswblank(int c);
#endif

int joe_wcwidth(unsigned int c);
/* Looking for wswidth? Take a look at scrn.c/txtwidth() */

int joe_towupper(int c);
int joe_towlower(int c);

extern unsigned char unictrlbuf[11];
int unictrl(unsigned int c);

#endif
