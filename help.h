/*
 *	Help system
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *		(C) 2001 Marek 'Marx' Grac
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#ifndef JUPP_HELP_H
#define JUPP_HELP_H

#ifdef EXTERN
__IDSTRING(rcsid_help_h, "$MirOS: contrib/code/jupp/help.h,v 1.10 2020/03/27 06:38:56 tg Exp $");
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
