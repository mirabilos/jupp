#!/bin/mksh
# $MirOS: contrib/code/jupp/autogen.sh,v 1.15 2017/12/02 17:13:33 tg Exp $
#-
# Copyright © 2004, 2005, 2006, 2008, 2017
#	mirabilos <m@mirbsd.org>
#
# Provided that these terms and disclaimer and all copyright notices
# are retained or reproduced in an accompanying document, permission
# is granted to deal in this work without restriction, including un‐
# limited rights to use, publicly perform, distribute, sell, modify,
# merge, give away, or sublicence.
#
# This work is provided “AS IS” and WITHOUT WARRANTY of any kind, to
# the utmost extent permitted by applicable law, neither express nor
# implied; without malicious intent or gross negligence. In no event
# may a licensor, author or contributor be held liable for indirect,
# direct, other damage, loss, or other issues arising in any way out
# of dealing in the work, even if advised of the possibility of such
# damage or existence of a defect, except proven that it results out
# of said person’s immediate fault when using the work as intended.

if [[ -z $AUTOCONF_VERSION ]]; then
	export AUTOCONF_VERSION=2.61
	print -u2 Warning: AUTOCONF_VERSION unset, using $AUTOCONF_VERSION!
fi

if [[ -z $AUTOMAKE_VERSION ]]; then
	export AUTOMAKE_VERSION=1.9
	print -u2 Warning: AUTOMAKE_VERSION unset, using $AUTOMAKE_VERSION!
fi

[[ -n $GNUSYSTEM_AUX_DIR ]] || GNUSYSTEM_AUX_DIR=/usr/src/gnu/share

export AUTOCONF_VERSION AUTOMAKE_VERSION GNUSYSTEM_AUX_DIR

AM_FLAGS=

for f in $files ChangeLog ltmain.sh; do
	[[ -e $f ]] && continue
	ln -s /dev/null $f
done

for f in libtool.m4 m4salt.inc m4sugar.inc; do
	[[ -s $f ]] || ln -sf "$GNUSYSTEM_AUX_DIR/$f" .
done

set -e
set -x
ACLOCAL_AMFLAGS=
[[ -e Makefile.am ]] && ACLOCAL_AMFLAGS=$(grep '^[:space:]*ACLOCAL_AMFLAGS' \
    Makefile.am | cut -d '=' -f 2)
aclocal -I . $ACLOCAL_AMFLAGS
f=configure.ac
[[ ! -e $f ]] && f=configure.in
[[ -n $NO_AUTOHEADER ]] || if fgrep -q \
    -e AC_CONFIG_HEADER -e AM_CONFIG_HEADER $f; then
	autoheader
fi
set +e
integer rv=0
[[ ! -e Makefile.am ]] || automake --foreign -a $AM_FLAGS || rv=$?
if autoconf; then
	chmod 664 configure
else
	(( rv = rv ? rv : 1 ))
fi
rm -rf autom4te.cache *~
find . -type l -print0 | xargs -0 rm
exit $rv
