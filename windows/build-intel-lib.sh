#!/bin/bash

set -e
if [[ "$MSYSTEM" == MINGW* ]]; then
    MINGW=1
    LIB=libbid.a
    LIB_OUT=libbid-mingw.a
else
    MINGW=0
    LIB=libbid.lib
    LIB_OUT=cl111libbid.lib
fi

if [ -f $LIB_OUT ]; then exit 0; fi

set -x
tar xfz ../inteldecimal/IntelRDFPMathLib20U1.tar.gz
cd IntelRDFPMathLib20U1
if (( $MINGW )); then
    patch -p0 --binary -i ../intel-lib-windows-32bit.patch
    cd LIBRARY
    # the second line of variables is to hack up Intel's insane makefiles that
    # don't understand mingw GNU toolchains on Windows.
    make CALL_BY_REF=1 GLOBAL_RND=1 GLOBAL_FLAGS=1 UNCHANGED_BINARY_FLAGS=0 \
         _HOST_OS=Windows_NT CC=gcc AR_CMD='ar rcs' AR_OUT= O=o A=a \
         lib
else
    patch -p0 <../intel-lib-windows-32bit.patch
    cd LIBRARY
    cmd /c ..\\..\\build-intel-lib.bat
fi

mv $LIB ../../$LIB_OUT
cd ../..
( echo '#ifdef FREE42_FPTEST'; echo 'const char *readtest_lines[] = {'; tr -d '\r' < IntelRDFPMathLib20U1/TESTS/readtest.in | sed 's/^\(.*\)$/"\1",/'; echo '0 };'; echo '#endif' ) > readtest_lines.cpp
cp IntelRDFPMathLib20U1/TESTS/readtest.c .
