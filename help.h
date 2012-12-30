/* $MirOS: contrib/code/jupp/help.h,v 1.3 2012/12/30 19:27:13 tg Exp $ */
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

#include "config.h"
#include "types.h"

struct help *help_actual;

void help_display PARAMS((SCREEN *t));		/* display text in help window */
void help_off PARAMS((SCREEN *t));		/* turn help off */
int help_on PARAMS((SCREEN *t));		/* turn help on */
int help_init PARAMS((unsigned char *filename));/* load help file */
struct help *find_context_help PARAMS((const unsigned char *name));

int u_help PARAMS((BASE *base));		/* toggle help on/off */
int u_helpcard PARAMS((BASE *base));		/* enable help at screen */
int u_help_next PARAMS((BASE *base));		/* goto next help screen */
int u_help_prev PARAMS((BASE *base));		/* goto prev help screen */

#endif
