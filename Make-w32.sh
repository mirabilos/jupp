#!/bin/mksh
# $MirOS: contrib/code/jupp/Make-w32.sh,v 1.2 2016/10/29 22:28:48 tg Exp $

export LC_ALL=C
set -ex
[[ -s configure && -s jupprc && -s charmaps/klingon.in ]]

jupp=$(sed -n "/^PACKAGE_VERSION='3.1jupp\([0-9]*\)[~'].*\$/s//\1/p" configure)
jwin=
while (( jupp > 34 )); do
	jwin=${jwin}z
	(( jupp -= 25 ))
done
typeset -i1 tmp
(( tmp = 1#a - 10 + jupp ))
jwin=$jwin${tmp#1#}
jtop=jwin31$jwin
typeset -u jWIN=$jwin

rm -rf mkw32
mkdir mkw32{,/{build,$jtop}}
cd mkw32/build
export CFLAGS='-Os -march=i486 -mtune=pentium-mmx'
mksh ../../configure \
    --prefix=c:/windows/system32 \
    --sysconfdir=c:/windows/system32 \
    --disable-dependency-tracking \
    --disable-curses \
    --disable-termcap \
    --disable-getpwnam \
    --disable-termidx \
    --enable-win32reloc=old
make
cp charmaps/* syntax/* ../$jtop/
cp jmacsrc joerc jpicorc jstarrc ../$jtop/
cp joe.exe ../$jtop/jupp32.exe
cd ../..
cp COPYING mkw32/$jtop/copying.txt
cp …/cygwin1.dll mkw32/$jtop/
cp joe.txt mkw32/$jtop/jupp32.txt
cp jupprc mkw32/$jtop/jupp32rc
sed "s!@jwin@!$jupp!g" <setup.inf >mkw32/$jtop/setup.inf
cd mkw32
chmod 444 $jtop/*
zip -D -X -9 -k ../JWIN31$jWIN.ZIP $jtop/*
cd ..
ls -l JWIN31$jWIN.ZIP
rm -rf mkw32
