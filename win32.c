/* $MirOS: contrib/code/jupp/win32.c,v 1.2 2016/10/30 02:38:35 tg Exp $ */

/*-
 * Copyright (c) 2016
 *	mirabilos <m@mirbsd.org>
 *
 * Provided that these terms and disclaimer and all copyright notices
 * are retained or reproduced in an accompanying document, permission
 * is granted to deal in this work without restriction, including un-
 * limited rights to use, publicly perform, distribute, sell, modify,
 * merge, give away, or sublicence.
 *
 * This work is provided "AS IS" and WITHOUT WARRANTY of any kind, to
 * the utmost extent permitted by applicable law, neither express nor
 * implied; without malicious intent or gross negligence. In no event
 * may a licensor, author or contributor be held liable for indirect,
 * direct, other damage, loss, or other issues arising in any way out
 * of dealing in the work, even if advised of the possibility of such
 * damage or existence of a defect, except proven that it results out
 * of said person's immediate fault when using the work as intended.
 *-
 * Retrieve the realpath of the program (what could be argv[0] if the
 * Unix designers had wanted it).
 */

#include <windows.h>

char *
cygwin32_argv0(void)
{
	DWORD res;
	/* plus one to detect truncation */
	char buf[MAX_PATH + 1];

	if (!(res = GetModuleFileName(NULL, buf, sizeof(buf))) ||
	    ((size_t)res > (size_t)(MAX_PATH)))
		return (NULL);
	buf[res] = '\0';
	return (strdup(buf));
}
