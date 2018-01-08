/*
 *	Help system
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *		(C) 2001 Marek 'Marx' Grac
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#ifndef _JOE_HELP_H
#define _JOE_HELP_H 1

#ifdef EXTERN
__IDSTRING(rcsid_help_h, "$MirOS: contrib/code/jupp/help.h,v 1.9 2018/01/08 00:40:44 tg Exp $");
#endif

extern struct help *help_actual;

void help_display(SCREEN *t);		/* display text in help window */
void help_off(SCREEN *t);		/* turn help off */
int help_on(SCREEN *t);			/* turn help on */
void help_init(const unsigned char *filename);	/* load help file */
struct help *find_context_help(const unsigned char *name);

int u_help(BASE *base);			/* toggle help on/off */
int u_helpcard(BASE *base);		/* enable help at screen */
int u_help_next(BASE *base);		/* goto next help screen */
int u_help_prev(BASE *base);		/* goto prev help screen */

#endif
