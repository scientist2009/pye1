#! /bin/sh
cd `dirname $0`
aclocal
autoheader
libtoolize --automake -f -c
automake --add-missing
autoconf
