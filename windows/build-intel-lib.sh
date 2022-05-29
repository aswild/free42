#!/bin/sh
set -eo pipefail

# don't let GNU MAKEFLAGS from MSYS environment confuse nmake
unset MAKEFLAGS

if [ -f cl111libbid.lib ]; then
    echo "cl111libbid.lib already built"
    exit 0
fi

set -x
tar xzf ../inteldecimal/IntelRDFPMathLib20U1.tar.gz
cd IntelRDFPMathLib20U1
patch -p0 --binary <../intel-lib-windows.patch

cd LIBRARY
cmd //c ..\\..\\build-intel-lib.bat
mv libbid.lib ../../cl111libbid.lib

cd ../..
cp IntelRDFPMathLib20U1/TESTS/readtest.c .
(
    echo '#ifdef FREE42_FPTEST'
    echo 'const char *readtest_lines[] = {'
    tr -d '\r' < IntelRDFPMathLib20U1/TESTS/readtest.in | sed 's/^\(.*\)$/"\1",/'
    echo 'NULL };'
    echo '#endif'
) > readtest_lines.cpp
