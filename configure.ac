# $MirOS: contrib/code/jupp/configure.ac,v 1.74 2018/11/11 18:20:51 tg Exp $
#-
# Process this file with autoconf to produce a configure script.

# We need AC_PROG_EGREP
AC_PREREQ(2.54)

#### Here's the only place where to change version number ####
AC_INIT(joe, 3.1jupp38)
#### But see main.c for the Copyright (c) owner and year! ####
AC_CONFIG_SRCDIR([b.c])

AC_CANONICAL_BUILD
AC_CANONICAL_HOST
AC_CANONICAL_TARGET

# Init automake
AM_INIT_AUTOMAKE
AM_MAINTAINER_MODE
AM_CONFIG_HEADER(autoconf.h)

# Checks for programs.
AC_PROG_CC
AC_PROG_CPP
AC_PROG_EGREP
AC_PROG_INSTALL
AC_PROG_LN_S

case $host in
*-*-minix*)
	CPPFLAGS="$CPPFLAGS -D_POSIX_SOURCE -D_POSIX_1_SOURCE=2 -D_MINIX"
	;;
*-gnu*)
	CPPFLAGS="$CPPFLAGS -D_GNU_SOURCE"
	;;
esac

case $host in
*-*-mirbsd* | *-*-openbsd*)
	AC_DEFINE(HAVE_GCC_ATTRIBUTE_BOUNDED, 1, [Define if GCC has Anil's bounds checker])
	;;
esac


# Checks for libraries.

search_libs_selinux="selinux"
search_libs_snprintf="snprintf db"
search_libs_term=""
search_libs_util="util"

AC_ARG_ENABLE(curses,
[  --disable-curses        disable use of curses library],
	curses=$enableval, curses=yes)
test x"$curses" = x"yes" && \
    search_libs_term="$search_libs_term tinfo ncursesw ncurses curses"

AC_ARG_ENABLE(termcap,
[  --disable-termcap       disable use of termcap library],
	termcap=$enableval, termcap=yes)
test x"$termcap" = x"yes" && \
    search_libs_term="$search_libs_term termcap"

AC_ARG_ENABLE(terminfo,
[  --disable-terminfo      disable use of terminfo functions],
	terminfo=$enableval, terminfo=yes)

AC_ARG_ENABLE(selinux,
[  --disable-selinux       disable SELinux context copying],
	selinux=$enableval, selinux=yes)

AC_ARG_ENABLE([search_libs],
[  --disable-search-libs   do not search in any external libraries],
	search_libs=$enableval, search_libs=yes)

if test x"$search_libs" = x"no"; then
	search_libs_selinux=""
	search_libs_snprintf=""
	search_libs_term=""
	search_libs_util=""
fi

AC_SEARCH_LIBS(openpty, $search_libs_util, AC_DEFINE_UNQUOTED(HAVE_OPENPTY, 1, [If we have BSD function openpty()]))
AC_SEARCH_LIBS(login_tty, $search_libs_util, AC_DEFINE_UNQUOTED(HAVE_LOGIN_TTY, 1, [If we have BSD function login_tty()]))
if test x"$terminfo" = x"no"; then
	echo "DISabling looking for terminal libraries entirely!"
else
	AC_SEARCH_LIBS(tgetflag, $search_libs_term, AC_DEFINE_UNQUOTED(TERMINFO, 1, [If we have newer terminfo/termcap capabilities]))
fi
AC_SEARCH_LIBS(snprintf, $search_libs_snprintf, AC_DEFINE_UNQUOTED(HAVE_SNPRINTF, 1, [If we have snprintf]))
if test x"$selinux" = x"no"; then
	echo "DISabling looking for SELinux support"
else
	AC_SEARCH_LIBS(is_selinux_enabled, $search_libs_selinux, AC_DEFINE_UNQUOTED(HAVE_SELINUX_FUN, 1, [We have SELinux functions]))
fi


# other conditionals

AC_ARG_ENABLE([getpwnam],
[  --disable-getpwnam      disable use of getpwnam function],
	getpwnam=$enableval, getpwnam=yes)
test x"$getpwnam" = x"no" && CPPFLAGS="$CPPFLAGS -DJOE_NOPWNAM"

AC_ARG_ENABLE([termidx],
[  --disable-termidx       do not build the termidx binary],
	termidx=$enableval, termidx=yes)
AM_CONDITIONAL([WANT_TERMIDX], [test x"$termidx" != x"no"])

AC_ARG_ENABLE([sysconfjoesubdir],
[  --enable-sysconfjoesubdir=/joe  (default value)],
	[sysconfjoesubdir=$enableval], [sysconfjoesubdir=yes])
if test x"$sysconfjoesubdir" = x"yes"; then
	sysconfjoesubdir=/joe
elif test x"$sysconfjoesubdir" = x"no"; then
	sysconfjoesubdir=
fi
AC_SUBST(sysconfjoesubdir)

AC_ARG_ENABLE([win32reloc],
[  --enable-win32reloc     enable win32 self-relocation],
	[win32reloc=$enableval], [win32reloc=no])
if test x"$win32reloc" = x"yes"; then
	JUPP_WIN32RELOC=1
else
	JUPP_WIN32RELOC=0
fi
AC_DEFINE_UNQUOTED([JUPP_WIN32RELOC], [$JUPP_WIN32RELOC], [Use Win32 argv0-relative paths?])
if test x"$win32reloc" = x"yes"; then
	joeinitfile='\\fBjoerc\\fR in the same directory as the binary resides in'
	joeinitcopy='which resides in the same directory as the binary'
	joeresides='C:\\eProgramme\\ejupp32\\ejoe.exe'
	jrcbefore=''
	jrcafter=' in the same directory as the binary'
elif test x"$win32reloc" = x"old"; then
	joeinitfile='\\fBc:/windows/system32/joe/joerc\\fR'
	joeinitcopy='which resides in \\fBc:/windows/system32/joe/joerc\\fR'
	joeresides='c:/windows/system32/joe.exe'
	jrcbefore='c:/windows/system32/joe/'
	jrcafter=''
else
	joeinitfile='usually \\fB'$sysconfdir$sysconfjoesubdir'/joerc\\fR'
	joeinitcopy='which normally resides in \\fB'$sysconfdir$sysconfjoesubdir'/joerc\\fR'
	joeresides=$bindir'/joe'
	jrcbefore=$sysconfdir$sysconfjoesubdir/
	jrcafter=''
fi
AC_SUBST(joeinitfile)
AC_SUBST(joeinitcopy)
AC_SUBST(joeresides)
AC_SUBST(jrcbefore)
AC_SUBST(jrcafter)


# Checks for header files.

AC_CHECK_HEADERS([ \
	sys/dirent.h sys/ioctl.h sys/param.h sys/stat.h sys/termio.h \
	sys/time.h sys/types.h sys/wait.h errno.h fcntl.h \
	langinfo.h limits.h locale.h paths.h pty.h pwd.h sgtty.h signal.h \
	stdlib.h string.h termio.h time.h unistd.h util.h utime.h \
	utmp.h bsd/string.h selinux/context.h selinux/selinux.h \
])
AC_HEADER_DIRENT
AC_SYS_POSIX_TERMIOS
if test "$ac_cv_sys_posix_termios" = "yes"; then
	AC_DEFINE(HAVE_POSIX_TERMIOS, 1, [We have <termios.h> and maybe <sys/termios.h>])
else
	if test "$ac_cv_header_termio_h" = "yes" -a "$ac_cvs_header_sys_termio_h" = "yes" ]; then
		AC_DEFINE(HAVE_SYSV_TERMIO, 1, [We have <termio.h> and <sys/termio.h>])
	fi
fi
AC_HEADER_TIOCGWINSZ

AC_HEADER_TIME

# Checks for typedefs, structures, and compiler characteristics.

AC_C_CONST
AC_SYS_LARGEFILE
if test "$cross_compiling" = "yes" ; then
	AC_MSG_WARN(cross compiling so assuming defaults for most 32 bit machines)
fi
AC_CHECK_SIZEOF(char, 1)
AC_CHECK_SIZEOF(short, 2)
AC_CHECK_SIZEOF(int, 4)
AC_CHECK_SIZEOF(long, 4)
AC_CHECK_SIZEOF(void *, 4)
AC_CHECK_SIZEOF(long long, 8)
AC_CHECK_SIZEOF(off_t, 8)
AC_CHECK_TYPE([sighandler_t],
	[AC_DEFINE([HAVE_SIGHANDLER_T], 1, [Define if you have the `sighandler_t' type.])],
	,
	[/* if you have signal.h header, include it */
#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif])

# Checks for library functions.

AC_PROG_GCC_TRADITIONAL
AC_FUNC_FSEEKO
AC_FUNC_LSTAT
joe_SETPGRP
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_CHECK_TYPE([ssize_t], [], [
	AC_DEFINE([ssize_t], [int], [Define to `int' if <sys/types.h> does not define.])
	], [])
AC_TYPE_SIGNAL
AC_FUNC_STAT
AC_CHECK_FUNCS([getcwd getwd])
if test x"$ac_cv_func_getcwd" != xyes; then
	if test x"$ac_cv_func_getwd" != xyes; then
		AC_MSG_ERROR([don't know how to get current working directory])
	fi
fi
AC_CHECK_FUNCS([alarm ctime fsync get_current_dir_name isatty mkdir mkstemp])
AC_CHECK_FUNCS([nl_langinfo popen putenv setitimer setlocale setpgid])
AC_CHECK_FUNCS([sigaction siginterrupt sigprocmask sigvec strchr strdup])
AC_CHECK_FUNCS([strlcat strlcpy sync utime])

AC_ARG_ENABLE([fork],
[  --disable-fork          disable use of fork],
	use_fork=$enableval, use_fork=yes)
test x"$use_fork" = x"no" && ac_cv_func_fork=no
AC_CHECK_FUNCS([fork unsetenv])
case $use_fork:$ac_cv_func_fork in
no:*) use_fork=0 ;;
*:yes) use_fork=1 ;;
*) use_fork=0 ;;
esac
AC_DEFINE_UNQUOTED([WANT_FORK], [$use_fork], [We have fork(2) and want to use it])
if test $use_fork = 1; then
	echo "Enabling use of fork(), concurrent subshells, etc."
else
	echo "DISabling use of fork() and concurrent subshells (NOMMU)!"
fi

AC_ARG_ENABLE([fpu],
[  --disable-fpu           disable use of floating point arithmetics],
	use_fpu=$enableval, use_fpu=yes)
if test x"$use_fpu" = x"no"; then
	use_fpu=0
else
	use_fpu=1
fi
AC_DEFINE_UNQUOTED([WANT_MATH], [$use_fpu], [Can we use the FPU?])
if test $use_fpu = 1; then
	echo "Enabling use of floating point arithmetics"
else
	echo "DISabling Math support for lack of an FPU!"
fi

AC_ARG_ENABLE([small],
[  --enable-small          reduce memory footprint by a few KiB],
	use_small=$enableval, use_small=no)
if test x"$use_small" = x"yes"; then
	# for constrained systems (like 8 MiB RAM total) only
	AC_DEFINE([SMALL], [1], [Reduce memory footprint slightly])
fi

AC_CHECK_DECLS([strlcpy, strlcat, popen])
AC_CHECK_DECLS([ctime], [], [], [AC_INCLUDES_DEFAULT
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
])

joe_REINSTALL_SIGHANDLERS

# Check for programs..
AC_CHECK_PROG(SPELL,aspell,aspell,ispell)

# strictly speaking, the .jsf files should have no substitutions,
# as they don’t do what you think they expand, currently it’s more
# expensive however to rename them in CVS, so we keep that until
# jupp has converted to mirtoconf
AC_CONFIG_FILES([Makefile
syntax/c.jsf syntax/perl.jsf syntax/verilog.jsf syntax/sh.jsf syntax/python.jsf
syntax/conf.jsf syntax/php.jsf syntax/mail.jsf syntax/pascal.jsf syntax/html.jsf
syntax/vhdl.jsf syntax/fortran.jsf syntax/java.jsf syntax/xml.jsf syntax/tcl.jsf
charmaps/klingon syntax/lisp.jsf syntax/csh.jsf syntax/mason.jsf syntax/diff.jsf
syntax/asm.jsf syntax/tex.jsf])
AC_OUTPUT
