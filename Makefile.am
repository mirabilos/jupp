# $MirOS: contrib/code/jupp/Makefile.am,v 1.9 2016/10/07 10:57:06 tg Exp $

AUTOMAKE_OPTIONS = foreign

EXTRA_DIST = COPYING INFO LIST NEWS README README.cvs TODO docs/help-system.html \
	joerc.in jmacsrc.in jstarrc.in rjoerc.in jpicorc.in jupprc \
	joe.1.in dir.c syntax/c.jsf.in syntax/perl.jsf.in syntax/verilog.jsf.in \
	syntax/conf.jsf.in syntax/python.jsf.in syntax/php.jsf.in syntax/sh.jsf.in \
	syntax/pascal.jsf.in syntax/html.jsf.in syntax/vhdl.jsf.in syntax/fortran.jsf.in \
	syntax/java.jsf.in syntax/xml.jsf.in syntax/tcl.jsf.in charmaps/klingon.in docs HINTS \
	syntax/lisp.jsf.in syntax/csh.jsf.in syntax/mail.jsf.in syntax/mason.jsf.in \
	syntax/diff.jsf.in syntax/asm.jsf.in syntax/tex.jsf.in

JOE_ALIASES = jmacs jpico jstar jupp rjoe

sysconfjoesubdir = /joe
sysconf_joedir = $(sysconfdir)$(sysconfjoesubdir)
sysconf_joe_DATA = joerc jmacsrc jstarrc rjoerc jpicorc jupprc

sysconf_syntaxdir = $(sysconf_joedir)/syntax
sysconf_syntax_DATA = syntax/c.jsf syntax/perl.jsf syntax/verilog.jsf \
	syntax/conf.jsf syntax/python.jsf syntax/php.jsf syntax/sh.jsf \
	syntax/mail.jsf syntax/pascal.jsf syntax/html.jsf syntax/vhdl.jsf \
	syntax/fortran.jsf syntax/java.jsf syntax/xml.jsf syntax/tcl.jsf \
	syntax/lisp.jsf syntax/csh.jsf syntax/mason.jsf syntax/diff.jsf \
	syntax/asm.jsf syntax/tex.jsf

sysconf_charmapsdir = $(sysconf_joedir)/charmaps
sysconf_charmaps_DATA = charmaps/klingon

man_MANS = joe.1
noinst_HEADERS = b.h blocks.h bw.h cmd.h config.h hash.h help.h kbd.h \
	macro.h main.h menu.h path.h poshist.h pw.h queue.h qw.h rc.h regex.h \
	scrn.h tab.h termcap.h tty.h tw.h types.h ublock.h uedit.h uerror.h \
	ufile.h uformat.h uisrch.h umath.h undo.h usearch.h ushell.h utag.h \
	utils.h va.h vfile.h vs.h w.h utf8.h syntax.h i18n.h charmap.h builtin.h


bin_PROGRAMS = joe
if WANT_TERMIDX
bin_PROGRAMS += termidx
endif

INCLUDES = -DJOERC="\"$(sysconf_joedir)/\""

# joe_LDADD = @REQRD_LIBS@

joe_SOURCES = b.c blocks.c bw.c cmd.c hash.c help.c kbd.c macro.c main.c menu.c \
	path.c poshist.c pw.c queue.c qw.c rc.c regex.c scrn.c tab.c termcap.c \
	tty.c tw.c ublock.c uedit.c uerror.c ufile.c uformat.c uisrch.c umath.c \
	undo.c usearch.c ushell.c utag.c va.c vfile.c vs.c w.c utils.c syntax.c \
	utf8.c selinux.c i18n.c charmap.c builtin.c builtins.c compat.c

termidx_SOURCES = termidx.c

install-exec-hook:
	cd $(DESTDIR)$(bindir) && for i in $(JOE_ALIASES); do \
		rm -f $$i; \
		$(LN_S) joe $$i; \
	done

install-man: install-man1-symlinks
install-man1-symlinks: install-man1
	cd $(DESTDIR)$(man1dir) && for i in $(JOE_ALIASES); do \
		rm -f $$i.1; \
		$(LN_S) joe.1 $$i.1; \
	done

uninstall-local:
	for i in $(JOE_ALIASES); do \
		rm -f $(DESTDIR)$(bindir)/$$i $(DESTDIR)$(man1dir)/$$i.1; \
	done

distclean-local:
	rm -f *~
	rm -f stamp-h.in charmaps/klingon syntax/asm.jsf syntax/c.jsf syntax/conf.jsf syntax/csh.jsf syntax/diff.jsf syntax/fortran.jsf syntax/html.jsf syntax/java.jsf syntax/lisp.jsf syntax/mail.jsf syntax/mason.jsf syntax/pascal.jsf syntax/perl.jsf syntax/php.jsf syntax/python.jsf syntax/sh.jsf syntax/tcl.jsf syntax/tex.jsf syntax/verilog.jsf syntax/vhdl.jsf syntax/xml.jsf

.PHONY: termcap
