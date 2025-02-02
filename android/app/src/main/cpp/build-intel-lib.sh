#!/bin/sh

set -eo pipefail

# figure out NDK version and path. build.gradle requests a specific version and we need to compile with
# that same version here outside of gradle. Mixing NDK versions in the same build could be bad.
NDK_VERSION="$(sed -nr 's/^[[:space:]]*ndkVersion[[:space:]]+"(.*)"[[:space:]]*$/\1/p' ../../../build.gradle)"
if [ -n "$NDK_VERSION" ]; then
    NDK="$ANDROID_HOME/ndk/$NDK_VERSION"
else
    # if no ndkVersion is set in build.gradle, then hope that ANDROID_NDK_ROOT is set to something sane
    NDK="${ANDROID_NDK_ROOT:-/opt/android-ndk}"
fi

# make sure we actually found an NDK
if [ -d "$NDK" ]; then
    echo "Found NDK path '$NDK' for version '$NDK_VERSION'"
else
    echo "ERROR: NDK version '$NDK_VERSION' not found in path '$NDK'"
    exit 1
fi

set -x

export PATH="$PWD/bin:$NDK/prebuilt/linux-x86_64/bin:$NDK/toolchains/llvm/prebuilt/linux-x86_64/bin:$PATH"
BUILT=0

make_wrapper() {
    cat >$2 <<EOF
#!/bin/sh
set -x
$1 "\$@"
EOF
    chmod +x $2
}

build_arch () {
    ARCH=$1
    ARCH_CC=$2
    ARCH_AR=$3
    ARCH_LIB=libgcc111libbid-$ARCH.a
    if [ -f $ARCH_LIB ]; then
        return
    fi
    rm -rf bin IntelRDFPMathLib20U1
    mkdir -p bin
    # Intel library doesn't like cross-compiling, it's makefiles are a clusterfuck
    # of functions and lists and abstractions, and so CC *must* be exactly "gcc"
    # or one of the other supported compilers. Thus, we need wrappers that call
    # the correct cross compiler.
    make_wrapper "$ARCH_CC" bin/gcc
    make_wrapper "$ARCH_AR" bin/ar

    if [ -f ../../../../../inteldecimal/IntelRDFPMathLib20U1.tar.gz ]
    then
        tar xfz ../../../../../inteldecimal/IntelRDFPMathLib20U1.tar.gz
    else
        tar xfz $HOME/free42/inteldecimal/IntelRDFPMathLib20U1.tar.gz
    fi

    cd IntelRDFPMathLib20U1
    patch -p1 <../intel-lib-android.patch
    cd LIBRARY
    make ANDROIDARCH=$ARCH CC=gcc CALL_BY_REF=1 GLOBAL_RND=1 GLOBAL_FLAGS=1 UNCHANGED_BINARY_FLAGS=0 _HOST_OS=Linux lib
    mv libbid.a ../../$ARCH_LIB
    cd ../..
    BUILT=1
}

#build_arch armv7 armv7a-linux-androideabi28-clang arm-linux-androideabi-ar
build_arch arm64 aarch64-linux-android34-clang llvm-ar
#build_arch x86 i686-linux-android28-clang i686-linux-android-ar
build_arch x86_64 x86_64-linux-android34-clang llvm-ar

if [ $BUILT -ne 0 ]; then
    cd IntelRDFPMathLib20U1/TESTS
    cp readtest.c readtest.h test_bid_conf.h test_bid_functions.h ../..
    cd ../..
    ( echo '#ifdef FREE42_FPTEST'; echo '#include <math.h>'; echo 'double log2(double x) { return log(x) / log(2); }'; echo 'const char *readtest_lines[] = {'; tr -d '\r' < IntelRDFPMathLib20U1/TESTS/readtest.in | sed 's/^\(.*\)$/"\1",/'; echo '0 };'; echo '#endif' ) > readtest_lines.cc
    rm -rf bin IntelRDFPMathLib20U1
fi
