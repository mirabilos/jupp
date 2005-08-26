#!/bin/mksh
# $MirOS: contrib/code/jupp/autogen.sh,v 1.6 2005/08/26 23:41:14 tg Exp $
#-
# Copyright (c) 2004, 2005
#	Thorsten "mirabile" Glaser <tg@66h.42h.de>
#
# Licensee is hereby permitted to deal in this work without restric-
# tion, including unlimited rights to use, publicly perform, modify,
# merge, distribute, sell, give away or sublicence, provided all co-
# pyright notices above, these terms and the disclaimer are retained
# in all redistributions or reproduced in accompanying documentation
# or other materials provided with binary redistributions.
#
# All advertising materials mentioning features or use of this soft-
# ware must display the following acknowledgement:
#	This product includes material provided by Thorsten Glaser.
#
# Licensor hereby provides this work "AS IS" and WITHOUT WARRANTY of
# any kind, expressed or implied, to the maximum extent permitted by
# applicable law, but with the warranty of being written without ma-
# licious intent or gross negligence; in no event shall licensor, an
# author or contributor be held liable for any damage, direct, indi-
# rect or other, however caused, arising in any way out of the usage
# of this work, even if advised of the possibility of such damage.

if [[ -z $AUTOCONF_VERSION ]]; then
	AUTOCONF_VERSION=2.59
	print Warning: AUTOCONF_VERSION unset!
fi

if [[ -z $AUTOMAKE_VERSION ]]; then
	AUTOMAKE_VERSION=1.9
	print Warning: AUTOMAKE_VERSION unset!
fi

[[ -n $GNUSYSTEM_AUX_DIR ]] || GNUSYSTEM_AUX_DIR=/usr/src/gnu/share

export AUTOCONF_VERSION AUTOMAKE_VERSION GNUSYSTEM_AUX_DIR

#AM_FLAGS="--miros --ignore-deps"
AM_FLAGS=""
[[ $AUTOMAKE_VERSION = 1.4 ]] && AM_FLAGS=
[[ -n $flags ]] && AM_FLAGS="$flags"

[[ -e /tmp/empty ]] || print -n >/tmp/empty
for a in $files ChangeLog ltmain.sh; do
	[[ -e $a ]] || ln -s /tmp/empty $a
done

set -e
set -x
[[ ! -e acinclude.m4 ]] || if [[ -d m4 ]]; then
	aclocal --acdir=/usr/local/share/aclocal-$AUTOMAKE_VERSION -I m4
elif [[ -d ../m4 ]]; then
	aclocal --acdir=/usr/local/share/aclocal-$AUTOMAKE_VERSION -I ../m4
else
	aclocal --acdir=/usr/local/share/aclocal-$AUTOMAKE_VERSION -I .
fi
f=configure.ac
[[ ! -e $f ]] && f=configure.in
fgrep -q -e AC_CONFIG_HEADER -e AM_CONFIG_HEADER $f && autoheader
set +e
let rv=0
[[ ! -e Makefile.am ]] || automake --foreign -a $AM_FLAGS || let rv=$?
autoconf && chmod 664 configure
rm -rf autom4te.cache
find . -type l -print0 | xargs -0 rm
exit $rv
