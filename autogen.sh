#!/bin/ksh
# $MirOS: contrib/code/jupp/autogen.sh,v 1.1 2005/02/05 02:42:20 tg Exp $
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
# of this work, even if advised of the possibility of such damage.

if [[ -z $AUTOCONF_VERSION ]]; then
	AUTOCONF_VERSION=2.59
	print Warning: AUTOCONF_VERSION unset!
fi

if [[ -z $AUTOMAKE_VERSION ]]; then
	AUTOMAKE_VERSION=1.9
	print Warning: AUTOMAKE_VERSION unset!
fi

export AUTOCONF_VERSION AUTOMAKE_VERSION

[[ -e ChangeLog ]] || ln -s NEWS ChangeLog

set -e
set -x
aclocal --acdir=/usr/local/share/aclocal-1.9 -I .
autoheader
set +e
automake --foreign -a --miros
autoconf && chmod 664 configure
[[ -e autom4te.cache ]] && rm -rf autom4te.cache
find . -type l -print0 | xargs -0 rm
exit 0
