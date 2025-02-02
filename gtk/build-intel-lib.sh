#!/bin/sh

if [ -z $MK ]; then
  which gmake >/dev/null 2>&1
  if [ $? -eq 0 ]; then
    MK=gmake
  else
    MK=make
  fi
fi

which gcc >/dev/null 2>&1
if [ $? -eq 0 ]; then
  CC=gcc
else
  CC=cc
fi

# don't let environment CFLAGS (intended for free42 code) pollute the build
unset CFLAGS

# Hack to support FreeBSD; not 100% sure what this does, but it produces a
# library that passes all tests.

if [ `uname -s` = "FreeBSD" ]; then
  OS_ARG="CFLAGS_OPT=-DLINUX"
else
  OS_ARG=
fi

# When building for big-endian targets, add "BID_BIG_ENDIAN=true" to the "make"
# command line. The library will work even if you don't, but the state files
# written by Free42 will only be compatible with those written on little-endian
# platforms if you do.
# Note that at the time of writing, I haven't created any successful big-endian
# builds yet. I have tried on Fedora 12 on Qemu emulating 32-bit PowerPC, and
# while the build succeeds and arithmetic and SQRT work, it crashes in the
# transcendentals.

if [ `echo ab | od -x | awk '{print $2}'` = "6162" ]; then
  ENDIAN_ARG="BID_BIG_ENDIAN=true"
else
  ENDIAN_ARG=
fi

# When building for architectures other than x86 or x86_64, I override the
# section titled "Determine host architecture" in
# IntelRDFPMathLib20U1/LIBRARY/makefile.iml_head, and replace it with a simple
# "_HOST_ARCH := x86" or "_HOST_ARCH := x86_64", depending on whether I'm
# building for a 32-bit or 64-bit platform, respectively. The actual CPU you
# specify seems to matter less than its word size. Thus, setting _HOST_ARCH
# to x86 works when targeting armv7 and ppc, both 32-bit platforms, and setting
# it to x86_64 works when targeting arm64, a 64-bit platform.
# Of course, proceed with caution. Your mileage may vary.
case `uname -m` in
  armv7|armv7l|i386|ppc)
      ARCH_ARG='_HOST_ARCH=x86'
    ;;
  arm64|aarch64|i86pc)
      ARCH_ARG='_HOST_ARCH=x86_64'
    ;;
  *)
      ARCH_ARG=
    ;;
esac

if [ -z "$V" ]; then
    V=0
fi

set -x
tar xzf ../inteldecimal/IntelRDFPMathLib20U1.tar.gz
cd IntelRDFPMathLib20U1
patch -p1 <../intel-lib-linux.patch

cd LIBRARY
$MK $OS_ARG $ARCH_ARG V="$V" CC="$CC" CALL_BY_REF=1 GLOBAL_RND=1 GLOBAL_FLAGS=1 UNCHANGED_BINARY_FLAGS=0 $ENDIAN_ARG lib

cd ../..
ln -sf IntelRDFPMathLib20U1/TESTS/readtest.c

set +x
echo "Generate readtest_lines.cc"
( echo '#ifdef FREE42_FPTEST'; echo 'const char *readtest_lines[] = {'; tr -d '\r' < IntelRDFPMathLib20U1/TESTS/readtest.in | sed 's/^\(.*\)$/"\1",/'; echo '0 };'; echo '#endif' ) > readtest_lines.cc
