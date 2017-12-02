# $MirOS: contrib/code/jupp/Makefile.am,v 1.17 2017/12/02 04:02:42 tg Exp $

AUTOMAKE_OPTIONS = foreign

EXTRA_DIST = \
	COPYING HINTS INFO LIST NEWS README README.cvs TODO \
	charmaps/klingon.in dir.c docs/help-system.html jmacsrc.in \
	joe.1.in joerc.in jpicorc.in jstarrc.in jupprc rjoerc.in \
	syntax/asm.jsf.in syntax/c.jsf.in syntax/conf.jsf.in \
	syntax/csh.jsf.in syntax/diff.jsf.in syntax/fortran.jsf.in \
	syntax/html.jsf.in syntax/java.jsf.in syntax/lisp.jsf.in \
	syntax/mail.jsf.in syntax/mason.jsf.in syntax/pascal.jsf.in \
	syntax/perl.jsf.in syntax/php.jsf.in syntax/python.jsf.in \
	syntax/sh.jsf.in syntax/tcl.jsf.in syntax/tex.jsf.in \
	syntax/verilog.jsf.in syntax/vhdl.jsf.in syntax/xml.jsf.in

JOE_ALIASES = jmacs jpico jstar jupp rjoe

sysconfjoesubdir = @sysconfjoesubdir@
sysconf_joedir = $(sysconfdir)$(sysconfjoesubdir)
sysconf_joe_DATA = jmacsrc joerc jpicorc jstarrc jupprc rjoerc

sysconf_syntaxdir = $(sysconf_joedir)/syntax
sysconf_syntax_DATA = \
	syntax/asm.jsf syntax/c.jsf syntax/conf.jsf syntax/csh.jsf \
	syntax/diff.jsf syntax/fortran.jsf syntax/html.jsf \
	syntax/java.jsf syntax/lisp.jsf syntax/mail.jsf syntax/mason.jsf \
	syntax/pascal.jsf syntax/perl.jsf syntax/php.jsf \
	syntax/python.jsf syntax/sh.jsf syntax/tcl.jsf syntax/tex.jsf \
	syntax/verilog.jsf syntax/vhdl.jsf syntax/xml.jsf

sysconf_charmapsdir = $(sysconf_joedir)/charmaps
sysconf_charmaps_DATA = charmaps/klingon

man_MANS = joe.1
noinst_HEADERS = \
	b.h blocks.h builtin.h bw.h charmap.h cmd.h config.h hash.h \
	help.h i18n.h kbd.h macro.h main.h menu.h path.h poshist.h pw.h \
	queue.h qw.h rc.h regex.h scrn.h syntax.h tab.h termcap.h tty.h \
	tw.h types.h ublock.h uedit.h uerror.h ufile.h uformat.h \
	uisrch.h umath.h undo.h usearch.h ushell.h utag.h utf8.h utils.h \
	va.h vfile.h vs.h w.h

bin_PROGRAMS = joe
if WANT_TERMIDX
bin_PROGRAMS += termidx
endif

if JUPP_WIN32RELOC
joe_WIN32_DEFS = -DJUPP_WIN32RELOC=1 -UJOERC
else
joe_WIN32_DEFS = -DJUPP_WIN32RELOC=0 -DJOERC="\"$(sysconf_joedir)/\""
endif

INCLUDES = $(joe_WIN32_DEFS)

joe_SOURCES = \
	b.c blocks.c builtin.c builtins.c bw.c charmap.c cmd.c compat.c \
	hash.c help.c i18n.c kbd.c macro.c main.c menu.c path.c \
	poshist.c pw.c queue.c qw.c rc.c regex.c scrn.c selinux.c \
	syntax.c tab.c termcap.c tty.c tw.c ublock.c uedit.c uerror.c \
	ufile.c uformat.c uisrch.c umath.c undo.c usearch.c ushell.c \
	utag.c utf8.c utils.c va.c vfile.c vs.c w.c win32.c

termidx_SOURCES = termidx.c

man_joeinitfile=@joeinitfile@
man_joeresides=@joeresides@
man_joeinitcopy=@joeinitcopy@
joe.1: joe.1.in
	sed \
	    -e 's!@''joeinitfile''@!$(man_joeinitfile)!g' \
	    -e 's!@''joeresides''@!$(man_joeresides)!g' \
	    -e 's!@''joeinitcopy''@!$(man_joeinitcopy)!g' \
	    <$? >$@~
	mv -f $@~ $@

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
