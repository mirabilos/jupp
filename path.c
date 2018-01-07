/*
 *	Directory and path functions
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#include "config.h"
#include "types.h"

__RCSID("$MirOS: contrib/code/jupp/path.c,v 1.23 2018/01/07 20:32:46 tg Exp $");

#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#ifdef HAVE_PATHS_H
#  include <paths.h>	/* for _PATH_TMP */
#endif
#include <stdlib.h>

#ifdef HAVE_BSD_STRING_H
#include <bsd/string.h>
#endif

#include "path.h"
#include "vs.h"
#include "va.h"

#ifdef TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# ifdef HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

#ifdef HAVE_DIRENT_H
#  include <dirent.h>
#  define NAMLEN(dirent) strlen((dirent)->d_name)
#else
#  ifdef HAVE_SYS_DIRENT_H
#    include <sys/dirent.h>
#    define NAMLEN(dirent) strlen((dirent)->d_name)
#  else
#    define direct dirent
#    define NAMLEN(dirent) (dirent)->d_namlen
#    ifdef HAVE_SYS_NDIR_H
#      include <sys/ndir.h>
#    else
#      ifdef HAVE_SYS_DIR_H
#        include <sys/dir.h>
#      else
#        ifdef HAVE_NDIR_H
#          include <ndir.h>
#        else
#          ifndef __MSDOS__
#            include "dir.c"
#          endif
#        endif
#      endif
#    endif
#  endif
#endif

#if HAVE_DRIVE_LETTERS
#define do_if_drive_letter(path, command) do {		\
	if ((path)[1] == ':') {				\
		drvltrhlprv = (path)[0] | 0x20;		\
		if (drvltrhlprv >= 'a' &&		\
		    drvltrhlprv <= 'z')			\
			command;			\
	}						\
} while (/* CONSTCOND */ 0)
#define drvltrhlpr unsigned char drvltrhlprv
#else
#define do_if_drive_letter(path, command) do { } while (/* CONSTCOND */ 0)
#define drvltrhlpr /* nothing */
#endif
#define skip_drive_letter(path)	do_if_drive_letter((path), (path) += 2)

#ifndef _PATH_TMP
#define _PATH_TMP	"/tmp/"
#endif

#if !defined(PATH_MAX) && !defined(HAVE_GET_CURRENT_DIR_NAME)
#warning What should we include to have PATH_MAX defined?
#define PATH_MAX	4096
#endif

/********************************************************************/
#if HAVE_BACKSLASH_PATHS
unsigned char *joesep(unsigned char *path)
{
	int x;

	for (x = 0; path[x]; ++x)
		if (path[x] == '\\')
			path[x] = '/';
	return path;
}
#endif
/********************************************************************/
unsigned char *namprt(unsigned char *path)
{
	unsigned char *z;
	drvltrhlpr;

	skip_drive_letter(path);
	z = path + slen(path);
	while ((z != path) && (z[-1] != '/'))
		--z;
	return vsncpy(NULL, 0, sz(z));
}
/********************************************************************/
unsigned char *namepart(unsigned char *tmp, unsigned char *path)
{
	unsigned char *z;
	drvltrhlpr;

	skip_drive_letter(path);
	z = path + strlen((char *)path);
	while ((z != path) && (z[-1] != '/'))
		--z;
	strlcpy((char *)tmp, (char *)z, 1024);
	return (tmp);
}
/********************************************************************/
unsigned char *dirprt_ptr(unsigned char *path)
{
	unsigned char *b = path;
	unsigned char *z = path + slen(path);
	drvltrhlpr;

	skip_drive_letter(b);
	while ((z != b) && (z[-1] != '/'))
		--z;
	return (z);
}
unsigned char *dirprt(unsigned char *path)
{
	return vsncpy(NULL, 0, path, dirprt_ptr(path) - path);
}
/********************************************************************/
unsigned char *begprt(unsigned char *path)
{
	unsigned char *z = path + slen(path);
	int drv = 0;
	drvltrhlpr;

	do_if_drive_letter(path, drv = 2);
	while ((z != path + drv) && (z[-1] == '/'))
		--z;
	if (z == path + drv)
		return vsncpy(NULL, 0, sz(path));
	else {
		while ((z != path + drv) && (z[-1] != '/'))
			--z;
		return vsncpy(NULL, 0, path, z - path);
	}
}
/********************************************************************/
unsigned char *endprt(unsigned char *path)
{
	unsigned char *z = path + slen(path);
	int drv = 0;
	drvltrhlpr;

	do_if_drive_letter(path, drv = 2);
	while ((z != path + drv) && (z[-1] == '/'))
		--z;
	if (z == path + drv)
		return vsncpy(NULL, 0, sc(""));
	else {
		while (z != path + drv && z[-1] != '/')
			--z;
		return vsncpy(NULL, 0, sz(z));
	}
}
/********************************************************************/
int mkpath(unsigned char *path)
{
	unsigned char *s;

	if (path[0] == '/') {
		if (chddir("/"))
			return 1;
		s = path;
		goto in;
	}

	while (path[0]) {
		int c;

		for (s = path; (*s) && (*s != '/'); s++) ;
		c = *s;
		*s = 0;
		if (chddir((char *)path)) {
			if (mkdir((char *)path, 0777))
				return 1;
			if (chddir((char *)path))
				return 1;
		}
		*s = c;
 in:
		while (*s == '/')
			++s;
		path = s;
	}
	return 0;
}
/********************************************************************/
/* Create a temporary file */
/********************************************************************/
unsigned char *mktmp(unsigned char *where, int *fdp)
{
#ifndef HAVE_MKSTEMP
	static unsigned seq = 0;
#endif
	unsigned char *name;
	int fd;
	unsigned namesize;

	if (!where)
		where = (unsigned char *)getenv("TMPDIR");
	if (!where)
		where = (unsigned char *)getenv("TEMP");
	if (!where)
		where = US _PATH_TMP;

	namesize = strlen((char *)where) + 20;
	name = vsmk(namesize);	/* [G.Ghibo'] we need to use vsmk() and not malloc() as
				   area returned by mktmp() is destroyed later with
				   vsrm(); */
#ifdef HAVE_MKSTEMP
	joe_snprintf_1((char *)name, namesize, "%s/joe.tmp.XXXXXXXXXX", where);
	if ((fd = mkstemp((char *)name)) == -1) {
		vsrm(name);
		/*
		 * FIXME: vflsh() and vflshf()
		 * expect mktmp() always succeed!
		 */
		return (NULL);
	}
	/*
	 * Linux glibc 2.0 mkstemp() creates it with 0666 mode, ergo we
	 * change it to 0600, so nobody else sees content of temporary file
	 */
	fchmod(fd, 0600);
#else
#warning "Waah, this is insane! Consider getting mkstemp!"
 loop:
	seq = (seq + 1) % 10000;
	joe_snprintf_3(name, namesize, "%s/joe.tmp.%04u%05u", where, seq,
	    (unsigned)(time(NULL) % 100000));
	if ((fd = open(name, O_RDONLY)) != -1) {
		close(fd);
		goto loop;	/* FIXME: possible endless loop --> DoS attack */
	}
	if ((fd = open(name, O_RDWR | O_CREAT | O_EXCL, 0600)) == -1) {
		vsrm(name);
		/* FIXME: see above */
		return (NULL);
	}
#endif
	if (fdp)
		*fdp = fd;
	else
		close(fd);
	return (name);
}
/********************************************************************/
int rmatch(const unsigned char *a, const unsigned char *b)
{
	int flag, inv, c;

	for (;;)
		switch (*a) {
		case '*':
			++a;
			do {
				if (rmatch(a, b))
					return 1;
			} while (*b++);
			return 0;
		case '[':
			++a;
			flag = 0;
			if (*a == '^') {
				++a;
				inv = 1;
			} else
				inv = 0;
			if (*a == ']')
				if (*b == *a++)
					flag = 1;
			while (*a && (c = *a++) != ']')
				if ((c == '-') && (a[-2] != '[') && (*a)) {
					if ((*b >= a[-2]) && (*b <= *a))
						flag = 1;
				} else if (*b == c)
					flag = 1;
			if ((!flag && !inv) || (flag && inv) || (!*b))
				return 0;
			++b;
			break;
		case '?':
			++a;
			if (!*b)
				return 0;
			++b;
			break;
		case 0:
			if (!*b)
				return 1;
			else
				return 0;
		default:
			if (*a++ != *b++)
				return 0;
		}
}
/********************************************************************/
int isreg(unsigned char *s)
{
	int x;

	for (x = 0; s[x]; ++x)
		if ((s[x] == '*') || (s[x] == '?') || (s[x] == '['))
			return 1;
	return 0;
}
/********************************************************************/
unsigned char **rexpnd(const unsigned char *word)
{
	void *dir;
	unsigned char **lst = NULL;

	struct dirent *de;
	dir = opendir(".");
	if (dir) {
		while ((de = readdir(dir)) != NULL)
			if (strcmp(".", de->d_name))
				if (rmatch(word, (unsigned char *)de->d_name))
					lst = vaadd(lst, vsncpy(NULL, 0, sz((unsigned char *)de->d_name)));
		closedir(dir);
	}
	return lst;
}
/********************************************************************/
int chJpwd(const unsigned char *path)
{
	unsigned char *fullpath;
	int rv;

	if (!has_JOERC)
		return (-1);
	fullpath = vsncpy(NULL, 0, sz(get_JOERC));
	fullpath = vsncpy(sv(fullpath), sz(path));
	rv = chpwd(fullpath);
	vsrm(fullpath);
	return (rv);
}

int chpwd(const unsigned char *path)
{
	if ((!path) || (!path[0]))
		return 0;
	return chdir((char *)path);
}

/* The pwd function */
unsigned char *pwd(void)
{
#if defined(PATH_MAX) || !defined(HAVE_GET_CURRENT_DIR_NAME)
	static unsigned char buf[PATH_MAX];
	unsigned char	*ret;

#ifdef HAVE_GETCWD
	ret = (unsigned char *)getcwd((char *)buf, PATH_MAX - 1);
#else
	ret = (unsigned char *)getwd((char *)buf);
#endif
	buf[PATH_MAX - 1] = '\0';

	return ret;
#else
	/* Hurd */
	static char *wd = NULL;

	free(wd);
	wd = get_current_dir_name();
	return ((void *)wd);
#endif
}

#if JUPP_WIN32RELOC
unsigned char has_JOERC = 0;
unsigned char *get_JOERC = NULL;

extern char *cygwin32_argv0(void);

void init_JOERC(void)
{
	struct stat sb;
	char *sep;

	if ((get_JOERC = (unsigned char *)cygwin32_argv0()) == NULL)
		return;
	joesep(get_JOERC);
	if ((sep = strrchr((char *)get_JOERC, '/')) == NULL)
		return;
	if (stat(get_JOERC, &sb))
		return;
	sep[1] = '\0';
	has_JOERC = 1;
}
#endif
