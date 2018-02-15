dnl $MirOS: contrib/code/jupp/acinclude.m4,v 1.4 2018/02/01 02:36:51 tg Exp $

dnl ------------------
dnl Check if setpgrp must have two arguments
dnl autoconf-own macro is damaged for *BSD systems
dnl ------------------
AC_DEFUN([joe_SETPGRP],
	[AC_CACHE_CHECK([whether setpgrp() takes no arguments],
		[joe_cv_setpgrp_void],
		[AC_TRY_RUN([
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
int main() {
	/* exit succesfully if setpgrp() takes two args (*BSD systems) */
	exit(setpgrp(0, 0) != 0);
}],
			[joe_cv_setpgrp_void=no],
			[joe_cv_setpgrp_void=yes],
			[joe_cv_setpgrp_void=yes])
		])
	if test "$joe_cv_setpgrp_void" = yes; then
		AC_DEFINE([SETPGRP_VOID], 1, [Define if setpgrp() takes no arguments])
	fi
])

dnl ------------------
dnl Check to see if signal handlers must be reinstalled when invoked
dnl ------------------
AC_DEFUN([joe_REINSTALL_SIGHANDLERS],
	[AC_CACHE_CHECK([whether sighandlers must be reinstalled],
		[joe_cv_reinstall_sighandlers],
		[AC_TRY_RUN([
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#if HAVE_SIGNAL_H
#include <signal.h>
#endif

#ifndef HAVE_SIGHANDLER_T
typedef RETSIGTYPE (*sighandler_t)(int);
#endif

int nsigint;

void set_signal(int signum, sighandler_t handler) {
#if HAVE_SIGACTION
        struct sigaction sact;

        sact.sa_handler = handler;
	sact.sa_flags = 0;
	sigemptyset(&sact.sa_mask);
        sigaction(signum, &sact, NULL);
#elif HAVE_SIGVEC
        struct sigvec svec;

        svec.sv_handler = handler;
	svec.sv_flags = 0;
	sigemptyset(&svec.sv_mask);
        sigvec(signum, &svec, NULL);
#else
        signal(signum, handler);
#endif
}

RETSIGTYPE sigint(int s) {
	nsigint++;
}

int main() {
	nsigint = 0;
	set_signal(SIGINT, sigint);
	kill((int)getpid(), SIGINT);
	kill((int)getpid(), SIGINT);
	/* exit succesfully if don't have to reinstall sighandler when invoked */
	exit(nsigint != 2);
}],
			[joe_cv_reinstall_sighandlers=no],
			[joe_cv_reinstall_sighandlers=yes],
			[joe_cv_reinstall_sighandlers=yes])
		])
	if test "$joe_cv_reinstall_sighandlers" = yes; then
		AC_DEFINE([NEED_TO_REINSTALL_SIGNAL], 1, [Define if we have to reinstall signal handler when invoked])
	fi
])
