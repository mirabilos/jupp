/*
 *	Menu selection window
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#ifndef JUPP_MENU_H
#define JUPP_MENU_H

#ifdef EXTERN_CMD_C
__IDSTRING(rcsid_menu_h, "$MirOS: contrib/code/jupp/menu.h,v 1.7 2020/03/27 06:38:57 tg Exp $");
#endif

/* Create a menu */
MENU *mkmenu(W *w, unsigned char **s, jpoly_int *func, jpoly_int *abrt, jpoly_int *backs, int cursor, void *object, int *notify);

/* Menu user functions */

int umuparw(MENU *m);
int umdnarw(MENU *m);
int umpgup(MENU *m);
int umpgdn(MENU *m);
int umltarw(MENU *m);
int umrtarw(MENU *m);
int umtab(MENU *m);
int umbof(MENU *m);
int umeof(MENU *m);
int umbol(MENU *m);
int umeol(MENU *m);
int umbacks(MENU *m);

void ldmenu(MENU *m, unsigned char **s, int cursor);

unsigned char *mcomplete(MENU *m);
unsigned char *find_longest(unsigned char **lst);

#endif
