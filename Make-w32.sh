#!/bin/mksh
# $MirOS: contrib/code/jupp/Make-w32.sh,v 1.1 2016/10/29 22:03:24 tg Exp $

set -ex
[[ -s configure && -s jupprc && -s charmaps/klingon.in ]]
export CFLAGS='-Os -march=i486 -mtune=pentium-mmx'
mksh configure \
    --prefix=c:/windows/system32 \
    --sysconfdir=c:/windows/system32 \
    --disable-dependency-tracking \
    --disable-curses \
    --disable-termcap \
    --disable-getpwnam \
    --disable-termidx \
    --enable-win32reloc=old
make
