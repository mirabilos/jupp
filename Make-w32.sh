#!/bin/mksh
# $MirOS: contrib/code/jupp/Make-w32.sh,v 1.9 2017/01/10 18:11:51 tg Exp $

extrawarnings="-Wall -Wextra"
if [[ $1 = -g ]]; then
	# Debug build and no packaging
	extrawarnings="$extrawarnings -g3"
fi
extrawarnings="$extrawarnings -Wno-unused-parameter"
echo "N: gcc-3.4.4-999 does not support -Wno-missing-field-initializers"
echo "N: expect warnings about those, they are known, do not report them"
extrawarnings="$extrawarnings -Wno-old-style-definition -Wno-strict-prototypes"
extrawarnings="$extrawarnings -Wno-cast-qual"
extrawarnings="$extrawarnings -Wno-missing-prototypes -Wno-missing-declarations"

export LC_ALL=C
set -ex
[[ -s configure && -s jupprc && -s charmaps/klingon.in ]]

jupp=$(sed -n "/^PACKAGE_VERSION='3\.1jupp\([0-9]*\)[~'].*\$/s//\1/p" configure)
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
export CPPFLAGS='-DJUPPRC_BUILTIN_NAME=\"jupp32rc\"'
mksh ../../configure \
    --prefix=c:/windows/system32 \
    --sysconfdir=c:/windows/system32 \
    --disable-dependency-tracking \
    --disable-curses \
    --disable-termcap \
    --disable-search-libs \
    --disable-getpwnam \
    --disable-termidx \
    --enable-win32reloc
make AM_CFLAGS="$extrawarnings"
if [[ $1 = -g ]]; then
	# Debug build with no packaging
	ln -s joe.exe jupp.exe
	ln -s ../../{charmaps,syntax,jmacsrc,joerc,jpicorc,jstarrc,jupprc} .
	exit 0
fi
cp charmaps/* syntax/* ../$jtop/
cp jmacsrc joerc jpicorc jstarrc ../$jtop/
cp joe.exe ../$jtop/jupp32.exe
cd ../..
cp COPYING mkw32/$jtop/copying.txt
cp /bin/cygwin1.dll mkw32/$jtop/
cp joe.txt mkw32/$jtop/jupp32.txt
cp jupprc mkw32/$jtop/jupp32rc
cd mkw32/$jtop
:>setup.inf
for x in *; do
	[[ $x = *[A-Z]* ]] || continue
	mv "$x" ../_TMP
	typeset -l lc
	lc=$x
	mv ../_TMP "$lc"
done
sed -b -e "s!@jwin@!$jupp!g" -e "s!@ts@!$(date -u +%m/%d/%Y)!g" \
    <../../setup.inf | while IFS= read -r line; do
	if [[ $line = '@files@'* ]]; then
		stat -c '%n=1,,%s' *
	else
		print -r -- "$line"
	fi
done >setup.inf
sed -bi "/^setup.inf=1,,/s/^.*\$/$(stat -c '%n=1,,%s' setup.inf)/" setup.inf
sed -bi "/^setup.inf=1,,/s/^.*\$/$(stat -c '%n=1,,%s' setup.inf)/" setup.inf
chmod 444 *
cd ..
zip -D -X -9 -k ../JWIN31$jWIN.ZIP $jtop/*
cd ..
ls -l JWIN31$jWIN.ZIP
rm -rf mkw32
