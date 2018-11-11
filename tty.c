/*
 *	UNIX Tty and Process interface
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#include "config.h"
#include "types.h"

__RCSID("$MirOS: contrib/code/jupp/tty.c,v 1.38 2018/11/11 18:20:51 tg Exp $");

#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <fcntl.h>

#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#if HAVE_UTMP_H
#include <utmp.h>
#endif

int idleout = 1;

/* We use the defines in sys/ioctl to determine what type
 * tty interface the system uses and what type of system
 * we actually have.
 */
#if defined(HAVE_POSIX_TERMIOS)
#include <termios.h>
#elif defined(HAVE_SYSV_TERMIO)
#include <termio.h>
#include <sys/termio.h>
#elif defined(HAVE_SGTTY_H)
#include <sgtty.h>
#endif

#ifdef HAVE_OPENPTY
#ifdef HAVE_PTY_H
#include <pty.h>
#endif
#ifdef HAVE_UTIL_H
#include <util.h>
#endif
#endif

/* Straight from the GNU autoconf texinfo documentation */
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

/* I'm not sure if SCO_UNIX and ISC have __svr4__ defined, but I think
   they might */
#ifdef M_SYS5
#ifndef M_XENIX
#include <sys/stream.h>
#include <sys/ptem.h>
#ifndef __svr4__
#define __svr4__ 1
#endif
#endif
#endif

#ifdef ISC
#ifndef __svr4__
#define __svr4__ 1
#endif
#endif

#ifdef __svr4__
#include <stropts.h>
#endif

/* JOE include files */

#include "main.h"
#include "path.h"
#include "scrn.h"
#include "tty.h"
#include "utils.h"
#include "ushell.h"

/** Aliased defines **/

/* O_NDELAY, O_NONBLOCK, and FNDELAY are all synonyms for placing a descriptor
 * in non-blocking mode; we make whichever one we have look like O_NDELAY
 */
#ifndef O_NDELAY
#ifdef O_NONBLOCK
#define O_NDELAY O_NONBLOCK
#endif
#ifdef FNDELAY
#define O_NDELAY FNDELAY
#endif
#endif

/* Some systems define this, some don't */
#ifndef sigmask
#define sigmask(x) (1<<((x)-1))
#endif

/* Some BSDs don't have TILDE */
#ifndef TILDE
#define TILDE 0
#endif

/* Global configuration variables */

int noxon = 0;			/* Set if ^S/^Q processing should be disabled */
int Baud = 0;			/* Baud rate from joerc, cmd line or environment */

/* The terminal */

FILE *termin = NULL;
FILE *termout = NULL;

/* Original state of tty */

#ifdef HAVE_POSIX_TERMIOS
struct termios oldterm;
#else /* HAVE_POSIX_TERMIOS */
#ifdef HAVE_SYSV_TERMIO
static struct termio oldterm;
#else /* HAVE_SYSV_TERMIO */
static struct sgttyb oarg;
static struct tchars otarg;
static struct ltchars oltarg;
#endif /* HAVE_SYSV_TERMIO */
#endif /* HAVE_POSIX_TERMIOS */

/* Output buffer, index and size */

unsigned char *obuf = NULL;
int obufp = 0;
int obufsiz;

/* The baud rate */

unsigned baud;			/* Bits per second */
unsigned long upc;		/* Microseconds per character */

/* TTY Speed code to baud-rate conversion table (this is dumb- is it really
 * too much to ask for them to just use an integer for the baud-rate?)
 */

static int speeds[] = {
	B50, 50, B75, 75, B110, 110, B134, 134, B150, 150, B200, 200,
	B300, 300, B600, 600,
	B1200, 1200, B1800, 1800, B2400, 2400, B4800, 4800, B9600, 9600
#ifdef EXTA
	    , EXTA, 19200
#endif
#ifdef EXTB
	    , EXTB, 38400
#endif
#ifdef B19200
	    , B19200, 19200
#endif
#ifdef B38400
	    , B38400, 38400
#endif
};

/* Input buffer */

int have = 0;			/* Set if we have pending input */
static unsigned char havec;	/* Character read in during pending input check */
int leave = 0;			/* When set, typeahead checking is disabled */

/* TTY mode flag.  1 for open, 0 for closed */
static int ttymode = 0;

/* Signal state flag.  1 for joe, 0 for normal */
static int ttysig = 0;

#if WANT_FORK
/* Stuff for shell windows */

static pid_t kbdpid;		/* PID of kbd client */
static int ackkbd = -1;		/* Editor acks keyboard client to this */

static int mpxfd;		/* Editor reads packets from this fd */
static int mpxsfd;		/* Clients send packets to this fd */

static int nmpx = 0;
static int tty_accept = NO_MORE_DATA;	/* =-1 if we have last packet */

struct packet {
	MPX *who;
	int size;
	int ch;
	unsigned char data[1024];
} pack;

MPX asyncs[NPROC];
#endif

/* Set signals for JOE */
void sigjoe(void)
{
	if (ttysig)
		return;
	ttysig = 1;
	joe_set_signal(SIGHUP, ttsig);
	joe_set_signal(SIGTERM, ttsig);
	joe_set_signal(SIGINT, SIG_IGN);
	joe_set_signal(SIGPIPE, SIG_IGN);
}

/* Restore signals for exiting */
void signrm(int inchild)
{
	if (!ttysig)
		return;
	if (!inchild)
		ttysig = 0;
	joe_set_signal(SIGHUP, SIG_DFL);
	joe_set_signal(SIGTERM, SIG_DFL);
	joe_set_signal(SIGINT, SIG_DFL);
	joe_set_signal(SIGPIPE, SIG_DFL);
}

/* Open terminal and set signals */

void ttopen(void)
{
	sigjoe();
	ttopnn();
}

/* Close terminal and restore signals */

void ttclose(void)
{
	ttclsn();
	signrm(0);
}

static volatile sig_atomic_t winched = 0;
#ifdef SIGWINCH
/* Window size interrupt handler */
static RETSIGTYPE winchd(int unused)
{
	winched = 1;
	REINSTALL_SIGHANDLER(SIGWINCH, winchd);
}
#endif

/* Second ticker */

static volatile sig_atomic_t ticked = 0;
extern int dostaupd;
static RETSIGTYPE dotick(int unused)
{
	ticked = 1;
}

void tickoff(void)
{
	alarm(0);
}

void tickon(void)
{
	ticked = 0;
	joe_set_signal(SIGALRM, dotick);
	alarm(1);
}

/* Open terminal */

static void baud_reset(int);

void ttopnn(void)
{
	int bbaud;

#ifdef HAVE_POSIX_TERMIOS
	struct termios newterm;
#else
#ifdef HAVE_SYSV_TERMIO
	struct termio newterm;
#else
	struct sgttyb arg;
	struct tchars targ;
	struct ltchars ltarg;
#endif
#endif

	if (!termin) {
		if (idleout ? (!(termin = stdin) || !(termout = stdout)) : (!(termin = fopen("/dev/tty", "r")) || !(termout = fopen("/dev/tty", "w")))) {
			fprintf(stderr, "Couldn\'t open /dev/tty\n");
			exit(1);
		} else {
#ifdef SIGWINCH
			joe_set_signal(SIGWINCH, winchd);
#endif
		}
	}

	if (ttymode)
		return;
	ttymode = 1;
	fflush(termout);

#ifdef HAVE_POSIX_TERMIOS
	tcgetattr(fileno(termin), &oldterm);
	newterm = oldterm;
	newterm.c_lflag = 0;
	if (noxon)
		newterm.c_iflag &= ~(ICRNL | IGNCR | INLCR | IXON | IXOFF);
	else
		newterm.c_iflag &= ~(ICRNL | IGNCR | INLCR);
	newterm.c_oflag = 0;
	newterm.c_cc[VMIN] = 1;
	newterm.c_cc[VTIME] = 0;
	tcsetattr(fileno(termin), TCSADRAIN, &newterm);
	bbaud = cfgetospeed(&newterm);
#else
#ifdef HAVE_SYSV_TERMIO
	ioctl(fileno(termin), TCGETA, &oldterm);
	newterm = oldterm;
	newterm.c_lflag = 0;
	if (noxon)
		newterm.c_iflag &= ~(ICRNL | IGNCR | INLCR | IXON | IXOFF);
	else
		newterm.c_iflag &= ~(ICRNL | IGNCR | INLCR);
	newterm.c_oflag = 0;
	newterm.c_cc[VMIN] = 1;
	newterm.c_cc[VTIME] = 0;
	ioctl(fileno(termin), TCSETAW, &newterm);
	bbaud = (newterm.c_cflag & CBAUD);
#else
	ioctl(fileno(termin), TIOCGETP, &arg);
	ioctl(fileno(termin), TIOCGETC, &targ);
	ioctl(fileno(termin), TIOCGLTC, &ltarg);
	oarg = arg;
	otarg = targ;
	oltarg = ltarg;
	arg.sg_flags = ((arg.sg_flags & ~(ECHO | CRMOD | XTABS | ALLDELAY | TILDE)) | CBREAK);
	if (noxon) {
		targ.t_startc = -1;
		targ.t_stopc = -1;
	}
	targ.t_intrc = -1;
	targ.t_quitc = -1;
	targ.t_eofc = -1;
	targ.t_brkc = -1;
	ltarg.t_suspc = -1;
	ltarg.t_dsuspc = -1;
	ltarg.t_rprntc = -1;
	ltarg.t_flushc = -1;
	ltarg.t_werasc = -1;
	ltarg.t_lnextc = -1;
	ioctl(fileno(termin), TIOCSETN, &arg);
	ioctl(fileno(termin), TIOCSETC, &targ);
	ioctl(fileno(termin), TIOCSLTC, &ltarg);
	bbaud = arg.sg_ospeed;
#endif
#endif
	baud_reset(bbaud);
}

static void
baud_reset(int bbaud)
{
	size_t x = 0;

	baud = 9600;
	upc = 0;
	while (x < NELEM(speeds))
		if (bbaud == speeds[x]) {
			baud = speeds[x + 1];
			break;
		} else
			x += 2;
	if (Baud >= 50)
		baud = Baud;
	else
		Baud = baud;
	upc = DIVIDEND / baud;
	if (obuf)
		free(obuf);
	if ((TIMES * upc) == 0)
		obufsiz = 4096;
	else {
		obufsiz = 1000000 / (TIMES * upc);
		if (obufsiz > 4096)
			obufsiz = 4096;
	}
	if (!obufsiz)
		obufsiz = 1;
	obuf = malloc(obufsiz);
}

/* Close terminal */

void ttclsn(void)
{
	int oleave;

	if (ttymode)
		ttymode = 0;
	else
		return;

	oleave = leave;
	leave = 1;

	ttflsh();

#ifdef HAVE_POSIX_TERMIOS
	tcsetattr(fileno(termin), TCSADRAIN, &oldterm);
#else
#ifdef HAVE_SYSV_TERMIO
	ioctl(fileno(termin), TCSETAW, &oldterm);
#else
	ioctl(fileno(termin), TIOCSETN, &oarg);
	ioctl(fileno(termin), TIOCSETC, &otarg);
	ioctl(fileno(termin), TIOCSLTC, &oltarg);
#endif
#endif

	leave = oleave;
}

/* Timer interrupt handler */

static volatile sig_atomic_t yep;
static RETSIGTYPE dosig(int unused)
{
	yep = 1;
}

/* FLush output and check for typeahead */

#ifdef HAVE_SETITIMER
#ifdef SIG_SETMASK
static void maskit(void)
{
	sigset_t set;

	sigemptyset(&set);
	sigaddset(&set, SIGALRM);
	sigprocmask(SIG_SETMASK, &set, NULL);
}

static void unmaskit(void)
{
	sigset_t set;

	sigemptyset(&set);
	sigprocmask(SIG_SETMASK, &set, NULL);
}

static void pauseit(void)
{
	sigset_t set;

	sigemptyset(&set);
	sigsuspend(&set);
}

#else
static void maskit(void)
{
	sigsetmask(sigmask(SIGALRM));
}

static void unmaskit(void)
{
	sigsetmask(0);
}

static void pauseit(void)
{
	sigpause(0);
}

#endif
#endif

int ttflsh(void)
{
	/* Flush output */
	if (obufp) {
		unsigned long usec = obufp * upc;	/* No. usecs this write should take */

#ifdef HAVE_SETITIMER
		if (usec >= 50000 && baud < 9600) {
			struct itimerval a, b;

			a.it_value.tv_sec = usec / 1000000;
			a.it_value.tv_usec = usec % 1000000;
			a.it_interval.tv_usec = 0;
			a.it_interval.tv_sec = 0;
			alarm(0);
			joe_set_signal(SIGALRM, dosig);
			yep = 0;
			maskit();
			setitimer(ITIMER_REAL, &a, &b);
			joe_write(fileno(termout), obuf, obufp);
			while (!yep)
				pauseit();
			unmaskit();
		} else
			joe_write(fileno(termout), obuf, obufp);

#else

		joe_write(fileno(termout), obuf, obufp);

#ifdef FIORDCHK
		if (baud < 9600 && usec / 1000)
			nap(usec / 1000);
#endif

#endif

		obufp = 0;
	}

#if WANT_FORK
	/* Ack previous packet */
	if (ackkbd != -1 && tty_accept != NO_MORE_DATA && !have) {
		unsigned char c = 0;

		if (pack.who && pack.who->func)
			joe_write(pack.who->ackfd, &c, 1);
		else
			joe_write(ackkbd, &c, 1);
		tty_accept = NO_MORE_DATA;
	}
#endif

	/* Check for typeahead or next packet */

	if (!have && !leave) {
#if WANT_FORK
		if (ackkbd != -1) {
			ssize_t r;

			fcntl(mpxfd, F_SETFL, O_NDELAY);
			r = read(mpxfd, &pack, 1);
			fcntl(mpxfd, F_SETFL, 0);
			if (r == 1) {
				r = sizeof(struct packet) - 1024 - 1;
				if (joe_readex(mpxfd, (US &pack) + 1, r) == r &&
				    pack.size >= 0 && pack.size <= 1024 &&
				    joe_readex(mpxfd, pack.data,
				    pack.size) == pack.size) {
					have = 1;
					tty_accept = pack.ch;
				}
			}
		} else
#endif
		  {
			/* Set terminal input to non-blocking */
			fcntl(fileno(termin), F_SETFL, O_NDELAY);

			/* Try to read */
			if (read(fileno(termin), &havec, 1) == 1)
				have = 1;

			/* Set terminal back to blocking */
			fcntl(fileno(termin), F_SETFL, 0);
		}
	}
	return 0;
}

/* Read next character from input */

#if WANT_FORK
static void mpxdied(MPX *m);
#endif

static time_t last_time;

int ttgetc(void)
{
	time_t new_time;

	tickon();

 loop:
	new_time = time(NULL);
	if (new_time != last_time) {
		last_time = new_time;
		dostaupd = 1;
		ticked = 1;
	}
	ttflsh();
	while (winched) {
		winched = 0;
		edupd(1);
		ttflsh();
	}
	if (ticked) {
		edupd(0);
		ttflsh();
		tickon();
	}
#if WANT_FORK
	if (ackkbd != -1) {
		ssize_t r;
		if (!have) {
			/* wait for input */
			r = sizeof(struct packet) - 1024;

			if (joe_readex(mpxfd, &pack, r) != r ||
			    pack.size < 0 || pack.size > 1024 ||
			    joe_readex(mpxfd, pack.data,
			    pack.size) != pack.size) {
				if (winched || ticked)
					goto loop;
				ttsig(0);
			}
			tty_accept = pack.ch;
		}
		have = 0;
		if (pack.who) {
			/* got background input */
			if (tty_accept != NO_MORE_DATA) {
				if (pack.who->func) {
					pack.who->func(pack.who->object, pack.data, pack.size);
					edupd(1);
				}
			} else
				mpxdied(pack.who);
			goto loop;
		} else if (tty_accept != NO_MORE_DATA) {
			tickoff();
			return tty_accept;
		} else {
			tickoff();
			ttsig(0);
			return 0;
		}
	}
#endif
	if (have) {
		have = 0;
	} else if (read(fileno(termin), &havec, 1) < 1) {
		if (winched || ticked)
			goto loop;
		ttsig(0);
	}
	tickoff();
	return havec;
}

/* Write string to output */

void ttputs(unsigned char *s)
{
	while (*s) {
		obuf[obufp++] = *s++;
		if (obufp == obufsiz)
			ttflsh();
	}
}

/* Get window size */

void ttgtsz(int *x, int *y)
{
#ifdef TIOCGSIZE
	struct ttysize getit;
#else
#ifdef TIOCGWINSZ
	struct winsize getit;
#endif
#endif

	*x = 0;
	*y = 0;

#ifdef TIOCGSIZE
	if (ioctl(fileno(termout), TIOCGSIZE, &getit) != -1) {
		*x = getit.ts_cols;
		*y = getit.ts_lines;
	}
#else
#ifdef TIOCGWINSZ
	if (ioctl(fileno(termout), TIOCGWINSZ, &getit) != -1) {
		*x = getit.ws_col;
		*y = getit.ws_row;
	}
#endif
#endif
}

#ifndef SIGTSTP
/* void ttshell(char *s);  Run a shell command or if 's' is zero, run a
 * sub-shell
 */
static void ttshell(unsigned char *cmd);
static const char shmsg[] =
    "You are at the command shell.  Type 'exit' to return\n";

#if WANT_FORK
#define v_or_fork() fork()
#else
#define v_or_fork() vfork()
#endif

static void ttshell(unsigned char *cmd)
{
	int x, omode = ttymode;
	const char *sh;

	sh = getushell();
	ttclsn();
	if (!(x = v_or_fork())) {
		signrm(1);
		if (cmd)
			execl(sh, sh, "-c", cmd, NULL);
		else {
			write(2, shmsg, sizeof(shmsg) - 1);
			execl(sh, sh, NULL);
		}
		_exit(0);
	}
	if (x != -1)
		wait(NULL);
	if (omode)
		ttopnn();
}
#endif

#if WANT_FORK
/* Create keyboard task */

static int mpxresume(void)
{
	int fds[2];

	if (pipe(fds)) {
		ackkbd = -1;
		return (1);
	}
	tty_accept = NO_MORE_DATA;
	have = 0;
	if (!(kbdpid = fork())) {
		close(fds[1]);
		do {
			unsigned char c;
			int sta;

			pack.who = 0;
			sta = joe_read(fileno(termin), &c, 1);
			if (sta == 0)
				pack.ch = NO_MORE_DATA;
			else
				pack.ch = c;
			pack.size = 0;
			joe_write(mpxsfd, &pack, sizeof(struct packet) - 1024);
		} while (joe_read(fds[0], &pack, 1) == 1);
		_exit(0);
	}
	close(fds[0]);
	ackkbd = fds[1];
	return (0);
}

/* Kill keyboard task */

static void mpxsusp(void)
{
	if (ackkbd!=-1) {
		kill(kbdpid, 9);
		while (wait(NULL) < 0 && errno == EINTR)
			/* do nothing */;
		close(ackkbd);
	}
}
#endif

/* We used to leave the keyboard copy task around during suspend, but
   Cygwin gets confused when two processes are waiting for input and you
   change the tty from raw to cooked (on the call to ttopnn()): the keyboard
   process was stuck in cooked until he got a carriage return- then he
   switched back to raw (he's supposed to switch to raw without waiting for
   the end of line). Probably this should be done for ttshell() as well. */

void ttsusp(void)
{
	int omode;

#ifdef SIGTSTP
	omode = ttymode;
#if WANT_FORK
	mpxsusp();
#endif
	ttclsn();
	fprintf(stderr, "You have suspended the program.  Type 'fg' to return\n");
	kill(0, SIGTSTP);
	if (omode)
		ttopnn();
#if WANT_FORK
	if (ackkbd!= -1)
		mpxresume();
#endif
#else
	ttshell(NULL);
#endif
}

#if WANT_FORK
/* Stuff for asynchronous I/O multiplexing.  We do not use streams or
   select() because joe needs to work on versions of UNIX which predate
   these calls.  Instead, when there is multiple async sources, we use
   helper processes which packetize data from the sources.  A header on each
   packet indicates the source.  There is no guarentee that packets getting
   written to the same pipe don't get interleaved, but you can reasonable
   rely on it with small packets. */

/* This code will explode if pipe, fork, etc. fail. --mirabilos */

static int mpxstart(void)
{
	int fds[2];

	if (pipe(fds)) {
		mpxfd = -1;
		mpxsfd = -1;
		return (1);
	}
	mpxfd = fds[0];
	mpxsfd = fds[1];
	return (mpxresume());
}

static void mpxend(void)
{
	mpxsusp();
	ackkbd = -1;
	close(mpxfd);
	close(mpxsfd);
	if (have)
		havec = pack.ch;
}

/* Get a pty/tty pair.  Returns open pty in 'ptyfd' and returns tty name
 * string in static buffer or NULL if couldn't get a pair.
 */

#ifdef __svr4__
#define USEPTMX 1
#else
#ifdef __CYGWIN__
#define USEPTMX 1
#endif
#endif

#ifdef sgi

/* Newer sgi machines can do it the __svr4__ way, but old ones can't */

extern char *_getpty(int *fildes, int oflag, mode_t mode, int nofork);

static unsigned char *getpty(int *ptyfd)
{
	return (unsigned char *)_getpty(ptyfd, O_RDWR, 0600, 0);
}

#else
#ifdef USEPTMX

/* Strange streams way */

extern char *ptsname(int);

static unsigned char *getpty(int *ptyfd)
{
	int fdm;

	*ptyfd = fdm = open("/dev/ptmx", O_RDWR);
	grantpt(fdm);
	unlockpt(fdm);
	return (unsigned char *)ptsname(fdm);
}

#else
#ifdef HAVE_OPENPTY

/* BSD function, present in libc5 and glibc2 and (duh) the BSDs */

static unsigned char *getpty(int *ptyfd)
{
	static unsigned char name[32];
	int ttyfd;

	if (openpty(ptyfd, &ttyfd, name, NULL, NULL) == 0)
	   return(name);
	else
	   return (NULL);
}

#else
/* The normal way: for each possible pty/tty pair, try to open the pty and
 * then the corresponding tty.  If both could be opened, close them both and
 * then re-open the pty.  If that succeeded, return with the opened pty and the
 * name of the tty.
 *
 * Logically you should only have to succeed in opening the pty- but the
 * permissions may be set wrong on the tty, so we have to try that too.
 * We close them both and re-open the pty because we want the forked process
 * to open the tty- that way it gets to be the controlling tty for that
 * process and the process gets to be the session leader.
 */

static unsigned char *getpty(int *ptyfd)
{
	int x, fd;
	unsigned char *orgpwd = pwd();
	static unsigned char **ptys = NULL;
	static const unsigned char *ttydir;
	static const unsigned char *ptydir;
	static unsigned char ttyname[32];

	if (!ptys) {
		/* HPUX systems */
		ttydir = UC "/dev/pty/";
		ptydir = UC "/dev/ptym/";
		if (chpwd(ptydir) || !(ptys = rexpnd(UC "pty*")))
			if (!ptys) {
				/* Everyone else */
				ttydir = ptydir = UC "/dev/";
				if (!chpwd(ptydir))
					ptys = rexpnd(UC "pty*");
			}
	}
	chpwd(orgpwd);

	if (ptys)
		for (fd = 0; ptys[fd]; ++fd) {
			strlcpy((char *)ttyname, (char *)ptydir, 32);
			strlcat((char *)ttyname, (char  *)(ptys[fd]), 32);
			if ((*ptyfd = open((char *)ttyname, O_RDWR)) >= 0) {
				ptys[fd][0] = 't';
				strlcpy((char *)ttyname, (char *)ttydir, 32);
				strlcat((char *)ttyname, (char *)(ptys[fd]), 32);
				ptys[fd][0] = 'p';
				x = open((char *)ttyname, O_RDWR);
				if (x >= 0) {
					close(x);
					close(*ptyfd);
					strlcpy((char *)ttyname, (char *)ptydir, 32);
					strlcat((char *)ttyname, (char *)(ptys[fd]), 32);
					*ptyfd = open((char *)ttyname, O_RDWR);
					ptys[fd][0] = 't';
					strlcpy((char *)ttyname, (char *)ttydir, 32);
					strlcat((char *)ttyname, (char *)(ptys[fd]), 32);
					ptys[fd][0] = 'p';
					return ttyname;
				} else
					close(*ptyfd);
			}
		}
	return NULL;
}

#endif
#endif
#endif

/* Shell dies signal handler.  Puts pty in non-block mode so
 * that read returns with <1 when all data from process has
 * been read. */
static volatile sig_atomic_t dead = 0;
int death_fd;
static RETSIGTYPE death(int unused)
{
	fcntl(death_fd,F_SETFL,O_NDELAY);
	wait(NULL);
	dead = 1;
}

#ifndef SIGCHLD
#define SIGCHLD SIGCLD
#endif

/* Build a new environment, but replace one variable */

extern unsigned char **mainenv;

static unsigned char **
newenv(unsigned char **old, const unsigned char *s)
{
	unsigned char **new;
	int x, y, z;

	for (x = 0; old[x]; ++x)
		/* nothing */;
	new = malloc((x + 2) * sizeof(unsigned char *));

	for (x = 0, y = 0; old[x]; ++x) {
		for (z = 0; s[z] != '='; ++z)
			if (s[z] != old[x][z])
				break;
		if (s[z] == '=') {
			if (s[z + 1])
				new[y++] = (void *)strdup((const char *)s);
		} else
			new[y++] = old[x];
	}
	if (x == y)
		new[y++] = (void *)strdup((const char *)s);
	new[y] = 0;
	return new;
}

/* Create a shell process */

MPX *
mpxmk(int *ptyfd, const unsigned char *cmd, unsigned char **args,
    void (*func)(B*, unsigned char *, int), void *object,
    void (*die)(B*), void *dieobj)
{
	unsigned char buf[80];
	int fds[2];
	int comm[2];
	pid_t pid;
	int x;
	MPX *m = NULL;
	unsigned char *name;

	/* Get pty/tty pair */
	if (!(name = getpty(ptyfd)))
		return NULL;

	/* Find free slot */
	for (x = 0; x != NPROC; ++x)
		if (!asyncs[x].func) {
			m = asyncs + x;
			break;
		}
	if (x==NPROC)
		return NULL;

	/* PID number pipe */
	if (pipe(comm))
		return (NULL);

	/* Acknowledgement pipe */
	if (pipe(fds)) {
		/* don't leak in error case */
 pipout:
		close(comm[0]);
		close(comm[1]);
		return (NULL);
	}
	m->ackfd = fds[1];

	/*
	 * Fixes cygwin console bug: if you fork() with inverse video
	 * it assumes you want ESC [ 0 m to keep it in inverse video
	 * from then on.
	 */
	set_attr(maint->t,0);

	/* Flush output */
	ttflsh();

	/* Bump no. current async inputs to joe */
	++nmpx;

	/* Start input multiplexer */
	if (ackkbd == -1)
		if (mpxstart()) {
			close(fds[0]);
			close(fds[1]);
			m->ackfd = -1;
			--nmpx;
			goto pipout;
		}

	/* Remember callback function */
	m->func = func;
	m->object = object;
	m->die = die;
	m->dieobj = dieobj;

	/* Create processes... */
	if (!(m->kpid = fork())) {
		/*
		 * This process copies data from shell to joe.
		 * After each packet it sends to joe it waits for
		 * an acknowledgement from joe so that it can not get
		 * too far ahead with buffering.
		 */

		/* Close joe side of pipes */
		close(fds[1]);
		close(comm[0]);

		/* Flag which indicates child died */
		dead = 0;
		death_fd = *ptyfd;
		joe_set_signal(SIGCHLD, death);

		if (!(pid = fork())) {
			/* This process becomes the shell */
			unsigned char **env;

			signrm(0);

			/* Close pty (we only need tty) */
			close(*ptyfd);

			/*
			 * All of this stuff is for dissociating ourself from
			 * the controlling tty (session leader) and starting a
			 * new session. This is the most non-portable part of
			 * UNIX — second only to pty/tty pair creation.
			 */
#ifndef HAVE_LOGIN_TTY

#ifdef TIOCNOTTY
			x = open("/dev/tty", O_RDWR);
			ioctl(x, TIOCNOTTY, 0);
#endif

			/* I think you do setprgp(0,0) on systems with no setsid() */
			setsid();
#ifndef _MINIX
/* http://mail-index.netbsd.org/pkgsrc-bugs/2011/06/13/msg043281.html */
#ifndef SETPGRP_VOID
			setpgrp(0, 0);
#else
			setpgrp();
#endif
#endif

#endif
			/* Close all fds */
			for (x = 0; x != 32; ++x) {
				/* Yes, this is quite a kludge... */
				/* All in the name of portability */
				close(x);
			}

			/* Open the TTY as standard input */
			if ((x = open((char *)name, O_RDWR)) != -1) {
				env = newenv(mainenv, UC "TERM=");

#ifdef HAVE_LOGIN_TTY
				login_tty(x);

#else
				/* This tells the fd that it's a tty (I think) */
#ifdef __svr4__
				ioctl(x, I_PUSH, "ptem");
				ioctl(x, I_PUSH, "ldterm");
#endif

				/* Open stdout, stderr */
				if (dup(x)) {}	/* standard output */
				if (dup(x)) {}	/* standard error */
				/*
				 * yes, stdin, stdout, and stderr must
				 * all be open for reading and writing.
				 * On some systems the shell assumes this.
				 */
#endif

				/*
				 * We could probably have a special TTY
				 * setup for JOE, but for now we'll just
				 * use the TTY setup for the TTY was was
				 * run on.
				 */
#ifdef HAVE_POSIX_TERMIOS
				tcsetattr(0, TCSADRAIN, &oldterm);
#else
#ifdef HAVE_SYSV_TERMIO
				ioctl(0, TCSETAW, &oldterm);
#else
				ioctl(0, TIOCSETN, &oarg);
				ioctl(0, TIOCSETC, &otarg);
				ioctl(0, TIOCSLTC, &oltarg);
#endif
#endif

				/* Execute the shell */
				execve((const char *)cmd, (char **)args, (char **)env);

				/* If shell didn't execute */
				joe_snprintf_1((char *)buf,sizeof(buf),"Couldn't execute shell '%s'\n",cmd);
				if (write(0,(char *)buf,strlen((char *)buf))) {}
				sleep(1);
			}

			_exit(0);
		}

		/* Tell JOE PID of shell */
		joe_write(comm[1], &pid, sizeof(pid));

		/* sigpipe should be ignored here. */

		/* This process copies data from shell to JOE until EOF.  It creates a packet
		   for each data */


		/* We don't really get EOF from a pty- it would just wait forever
		   until someone else writes to the tty.  So: when the shell
		   dies, the child died signal handler death() puts pty in non-block
		   mode.  This allows us to read any remaining data- then
		   read returns 0 and we know we're done. */

 loop:
		pack.who = m;
		pack.ch = 0;

		/* Read data from process */
		pack.size = joe_read(*ptyfd, pack.data, 1024);

		/* On SUNOS 5.8, the very first read from the pty returns 0 for some reason */
		if (!pack.size)
			pack.size = joe_read(*ptyfd, pack.data, 1024);

		if (pack.size > 0) {
			/* Send data to JOE, wait for ack */
			joe_write(mpxsfd, &pack, sizeof(struct packet) - 1024 + pack.size);

			joe_read(fds[0], &pack, 1);
			goto loop;
		} else {
			/* Shell died: return */
			pack.ch = NO_MORE_DATA;
			pack.size = 0;
			joe_write(mpxsfd, &pack, sizeof(struct packet) - 1024);

			_exit(0);
		}
	}
	joe_read(comm[0], &m->pid, sizeof(m->pid));

	/* We only need comm once */
	close(comm[0]);
	close(comm[1]);

	/* Close other side of copy process pipe */
	close(fds[0]);
	return m;
}

static void mpxdied(MPX *m)
{
	if (!--nmpx)
		mpxend();
	while (wait(NULL) < 0 && errno == EINTR)
		/* do nothing */;
	if (m->die)
		m->die(m->dieobj);
	m->func = NULL;
	edupd(1);
}
#endif

void
tty_xonoffbaudrst(void)
{
#ifdef HAVE_POSIX_TERMIOS
	struct termios newterm;
#else
#ifdef HAVE_SYSV_TERMIO
	struct termio newterm;
#else
	struct sgttyb arg;
	struct tchars targ;
#endif
#endif

#ifdef HAVE_POSIX_TERMIOS
	tcgetattr(fileno(termin), &newterm);
	if (noxon)
		newterm.c_iflag &= ~(IXON | IXOFF);
	else
		newterm.c_iflag |= (IXON | IXOFF);
	tcsetattr(fileno(termin), TCSADRAIN, &newterm);
	baud_reset(cfgetospeed(&newterm));
#else
#ifdef HAVE_SYSV_TERMIO
	ioctl(fileno(termin), TCGETA, &newterm);
	if (noxon)
		newterm.c_iflag &= ~(IXON | IXOFF);
	else
		newterm.c_iflag |= (IXON | IXOFF);
	ioctl(fileno(termin), TCSETAW, &newterm);
	baud_reset(newterm.c_cflag & CBAUD);
#else
	ioctl(fileno(termin), TIOCGETP, &arg);
	ioctl(fileno(termin), TIOCGETC, &targ);
	if (noxon) {
		targ.t_startc = -1;
		targ.t_stopc = -1;
	} else {
		targ.t_startc = otarg.t_startc;
		targ.t_stopc = otarg.t_stopc;
	}
	ioctl(fileno(termin), TIOCSETC, &targ);
	baud_reset(arg.sg_ospeed);
#endif
#endif
}
