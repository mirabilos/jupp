#!/bin/ksh
# $MirBSD: autogen.sh,v 1.1 2004/11/10 21:03:22 tg Exp $
# _MirBSD: contrib/gnu/aux/autogen.sh,v 1.2 2004/10/16 23:58:04 tg Exp $
#-
# Copyright (c) 2004
#	Thorsten "mirabile" Glaser <tg@66h.42h.de>
#
# Licensee is hereby permitted to deal in this work without restric-
# tion, including unlimited rights to use, publicly perform, modify,
# merge, distribute, sell, give away or sublicence, provided all co-
# pyright notices above, these terms and the disclaimer are retained
# in all redistributions or reproduced in accompanying documentation
# or other materials provided with binary redistributions.
#
# Licensor hereby provides this work "AS IS" and WITHOUT WARRANTY of
# any kind, expressed or implied, to the maximum extent permitted by
# applicable law, but with the warranty of being written without ma-
# licious intent or gross negligence; in no event shall licensor, an
# author or contributor be held liable for any damage, direct, indi-
# rect or other, however caused, arising in any way out of the usage
# of covered work, even if advised of the possibility of such damage.

if [[ -z $AUTOCONF_VERSION ]]; then
	export AUTOCONF_VERSION=2.59
	print Warning: AUTOCONF_VERSION unset!
fi

set -e
set -x
aclocal -I .
autoheader
set +e
automake --foreign -i
autoconf && chmod 664 configure
[[ -e autom4te.cache ]] && rm -rf autom4te.cache
