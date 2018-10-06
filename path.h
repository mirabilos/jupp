/*
 *	Directory and path functions
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#ifndef _JOE_PATH_H
#define _JOE_PATH_H 1

#ifdef EXTERN
__IDSTRING(rcsid_path_h, "$MirOS: contrib/code/jupp/path.h,v 1.17 2018/03/15 22:48:01 tg Exp $");
#endif

#if defined(__MSDOS__) || defined(__DJGPP__) || defined(__EMX__) || \
    defined(__CYGWIN__) || defined(_WIN32)
/*XXX check this for all platforms */
#define HAVE_BACKSLASH_PATHS 1
#define HAVE_DRIVE_LETTERS 1
#else
#define HAVE_BACKSLASH_PATHS 0
#define HAVE_DRIVE_LETTERS 0
#endif

#if HAVE_BACKSLASH_PATHS
unsigned char *joesep(unsigned char *path);
#else
#define joesep(path) (path)
#endif

#if JUPP_WIN32RELOC
#undef JOERC
extern unsigned char has_JOERC, *get_JOERC;
void init_JOERC(void);
#else
#define has_JOERC	1
#define get_JOERC	JOERC
#define init_JOERC()	/* nothing */
#endif

/* char *namprt(char *path);
 * Return name part of a path.  There is no name if the last character
 * in the path is '/'.
 *
 * The name part of "/hello/there" is "there"
 * The name part of "/hello/" is ""
 * The name part if "/" is ""
 */
unsigned char *namprt(unsigned char *path);
unsigned char *namepart(unsigned char *tmp, unsigned char *path)
    ATTR_BOUNDED((__minbytes__,1,1024));

/* char *dirprt(char *path);
 * Return directory and drive part of a path.  I.E., everything to the
 * left of the name part.
 *
 * The directory part of "/hello/there" is "/hello/"
 * The directory part of "/hello/" is "/hello/"
 * The directory part of "/" is "/"
 *
 * dirprt_ptr points to just beyond what dirprt returns.
 */
unsigned char *dirprt(unsigned char *path);
unsigned char *dirprt_ptr(unsigned char *path);

/* char *begprt(char *path);
 * Return the beginning part of a path.
 *
 * The beginning part of "/hello/there" is "/hello/"
 * The beginning part of "/hello/" is "/"
 * The beginning part of "/" is "/"
 */
unsigned char *begprt(unsigned char *path);

/* char *endprt(char *path);
 * Return the ending part of a path.
 *
 * The ending part of "/hello/there" is "there"
 * The ending part of "/hello/" is "hello/"
 * The ending part of "/" is ""
 */
unsigned char *endprt(unsigned char *path);

/* int mkpath(char *path);
 * Make sure path exists.  If it doesn't, try to create it
 *
 * Returns 1 for error or 0 for success.  The current directory
 * and drive will be at the given path if successful, otherwise
 * the drive and path will be elsewhere (not necessarily where they
 * were before mkpath was called).
 */
int mkpath(unsigned char *path);

/* char *mktmp(char *, int *);
 * Create an empty temporary file.  The file name created is the string passed
 * to this function postfixed with /joe.tmp.XXXXXX, where XXXXXX is some
 * string six chars long which makes this file unique.
 * If second argument is not NULL, fd is kept open and stored there.
*/
unsigned char *mktmp(unsigned char *where, int *fdp);

/* Change drive and directory */
#define chddir chdir

/* int rmatch(char *pattern,char *string);
 * Return true if string matches pattern
 *
 * Pattern is:
 *     *                 matches 0 or more charcters
 *     ?                 matches any 1 character
 *     [...]             matches 1 character in set
 *     [^...]            matches 1 character not in set
 *     [[]               matches [
 *     [*]               matches *
 *     [?]               matches ?
 *     any other         matches self
 *
 *  Ranges of characters may be specified in sets with 'A-B'
 *  '-' may be specified in sets by placing it at the ends
 *  '[' may be specified in sets by placing it first
 */
int rmatch(const unsigned char *a, const unsigned char *b);
int isreg(unsigned char *s);

/* char **rexpnd(char *path,char *pattern);
 * Generate array (see va.h) of file names from directory in 'path'
 * which match the pattern 'pattern'
 */
unsigned char **rexpnd(const unsigned char *word);

int chJpwd(const unsigned char *path);
int chpwd(const unsigned char *path);
unsigned char *pwd(void);

#endif
