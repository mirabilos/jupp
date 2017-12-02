/*
 *	Command execution
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#ifndef _JOE_CMD_H
#define _JOE_CMD_H 1

#ifdef EXTERN_CMD_C
__IDSTRING(rcsid_cmd_h, "$MirOS: contrib/code/jupp/cmd.h,v 1.6 2017/12/02 17:00:48 tg Exp $");
#endif

extern CMD cmds[];		/* Built-in commands */
extern int dobeep;

/* Command execution flags */

#define EMID		  1	/* Recenter screen */
#define ECHKXCOL	  2	/* Don't execute command if cursor column is wrong */
#define EFIXXCOL	  4	/* Fix column position after command has executed */
#define EMINOR		  8	/* Full screen update not needed */
#define EPOS		 16	/* A position history command */
#define EMOVE		 32	/* A movement for position history purposes */
#define EKILL		 64	/* Function is a kill */
#define EMOD		128	/* Not allowed on readonly files */
/* These use same bits as TYPE* in types.h */
#define EBLOCK		0x4000	/* Finish block selection */

/* CMD *findcmd(char *s);
 * Return command address for given name
 */
CMD *findcmd PARAMS((unsigned char *s));
void addcmd PARAMS((unsigned char *s, MACRO *m));

/* Execute a command.  Returns return value of command */
int execmd PARAMS((CMD *cmd, int k));

#endif
