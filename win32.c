/*-
 * Copyright (c) 2016, 2017
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
 */

#include "config.h"

__RCSID("$MirOS: contrib/code/jupp/win32.c,v 1.4 2017/12/02 02:07:39 tg Exp $");

#ifdef __CYGWIN__

#include <cygwin/version.h>
#include <windows.h>

#if JUPP_WIN32RELOC
/*
 * Retrieve the realpath of the program (what could be argv[0] if the
 * Unix designers had wanted it).
 */
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
#endif

/* return command line as passed to the .EXE (just like cygwin32’s dcrt0.cc) */
unsigned char *
cygwin32_cmdline(void)
{
	char *cp;

	cp = strdup(GetCommandLineA());
	if (!AreFileApisANSI())
		CharToOemA(cp, cp);
	return ((unsigned char *)cp);
}

/* Cygwin before 1.7.2 did not have locale support */
#if defined(CYGWIN_VERSION_API_MAJOR) && (CYGWIN_VERSION_API_MAJOR < 1) && \
    defined(CYGWIN_VERSION_API_MINOR) && (CYGWIN_VERSION_API_MINOR < 222)
/*
 * Mirror get_cp() in winsup/cygwin/miscfuncs.cc as used by
 * dev_console::str_to_con() in winsup/cygwin/fhandler_console.cc
 */
unsigned int
cygwin32_get_cp(void)
{
	return (AreFileApisANSI() ? GetACP() : GetOEMCP());
}
#endif

#endif /* __CYGWIN__ */
