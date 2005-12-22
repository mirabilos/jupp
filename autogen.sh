#!/bin/mksh
# $MirOS: contrib/code/jupp/autogen.sh,v 1.8 2005/12/22 20:18:34 tg Exp $
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
# Licensor offers the work "AS IS" and WITHOUT WARRANTY of any kind,
# express, or implied, to the maximum extent permitted by applicable
# law, without malicious intent or gross negligence; in no event may
# licensor, an author or contributor be held liable for any indirect
# or other damage, or direct damage except proven a consequence of a
# direct error of said person and intended use of this work, loss or
# other issues arising in any way out of its use, even if advised of
# the possibility of such damage or existence of a nontrivial bug.

self=$(readlink -f $(dirname $0))

if [[ -z $AUTOCONF_VERSION ]]; then
	AUTOCONF_VERSION=2.59
	print Warning: AUTOCONF_VERSION unset!
fi

if [[ -z $AUTOMAKE_VERSION ]]; then
	AUTOMAKE_VERSION=1.9
	print Warning: AUTOMAKE_VERSION unset!
fi

export AUTOCONF_VERSION AUTOMAKE_VERSION

[[ -n $GNUSYSTEM_AUX_DIR ]] || GNUSYSTEM_AUX_DIR=/usr/src/gnu/share
export GNUSYSTEM_AUX_DIR

#AM_FLAGS=--miros
AM_FLAGS=
[[ $AUTOMAKE_VERSION = 1.4 ]] && AM_FLAGS=
[[ -n $flags ]] && AM_FLAGS=$flags

[[ -e /tmp/empty ]] || print -n >/tmp/empty
for a in $files ChangeLog ltmain.sh; do
	[[ -e $a ]] || ln -s /tmp/empty $a
done

set -e
set -x
if [[ -d m4 ]]; then
	aclocal --acdir=/usr/local/share/aclocal-$AUTOMAKE_VERSION -I m4
elif [[ -d ../m4 ]]; then
	aclocal --acdir=/usr/local/share/aclocal-$AUTOMAKE_VERSION -I ../m4
else
	aclocal --acdir=/usr/local/share/aclocal-$AUTOMAKE_VERSION -I .
fi
autoheader
set +e
automake --foreign -a $AM_FLAGS
autoconf && chmod 664 configure
[[ -e autom4te.cache ]] && rm -rf autom4te.cache
find . -type l -print0 | xargs -0 rm
exit 0
