#!/bin/bash

WORKSPACE=$(pwd)

function checkReturn() {
    if [ "$1" != "0" ]; then
        echo "Error on line ${BASH_LINENO[0]}: $1"
        exit $1
    fi
}

AUTO_INSTALL_TOOLS=${AUTO_INSTALL_TOOLS:-true}
if [ "${AUTO_INSTALL_TOOLS}" == "true" ]; then
    echo "=== Installing prerequisite tools ==="
    DEBIAN_FRONTEND=noninteractive apt update && apt install -y git make autoconf libtool android-tools-adb unzip wget
    checkReturn $?
fi

# https://developer.android.com/ndk/downloads/
export ANDROID_NDK_ROOT=${ANDROID_NDK_ROOT:-${WORKSPACE}/android-ndk-r26b}
if [ ! -e ${ANDROID_NDK_ROOT} ]; then
    echo "=== Installing Android NDK ==="
    wget -q https://dl.google.com/android/repository/android-ndk-r26b-linux.zip && \
        unzip android-ndk-r26b-linux.zip
    checkReturn $?
fi
PATH="${ANDROID_NDK_ROOT}/toolchains/llvm/prebuilt/linux-x86_64/bin:$PATH"

if [ "${CLEAN_BUILD}" = "true" ]; then
    rm -rf ${WORKSPACE}/openssl-* ${WORKSPACE}/wolfssl-*
fi

# Compile OpenSSL
if [ ! -e ${WORKSPACE}/openssl-install ]; then
    OPENSSL_BRANCH=${OPENSSL_BRANCH:-"master"}
    echo "=== Installing OpenSSL ==="
    export OPENSSL_ALL_CIPHERS="-cipher ALL -ciphersuites TLS_AES_256_GCM_SHA384:TLS_AES_128_GCM_SHA256:TLS_AES_128_CCM_SHA256:TLS_AES_128_CCM_8_SHA256"
    git clone https://github.com/openssl/openssl.git --branch=${OPENSSL_BRANCH} ${WORKSPACE}/openssl-source && \
        cd ${WORKSPACE}/openssl-source && \
        ./Configure android-x86_64 no-sm3 no-sm4 --prefix=${WORKSPACE}/openssl-install && \
        sed -i 's/-ldl//g' Makefile && \
        sed -i 's/-pie//g' Makefile && \
        make -j && \
        make -j install
    checkReturn $?
fi
export LD_LIBRARY_PATH="${WORKSPACE}/openssl-install/lib64:$LD_LIBRARY_PATH"

# Compile WolfSSL
export UNAME=Android
export CROSS_COMPILE=${ANDROID_NDK_ROOT}/toolchains/llvm/prebuilt/linux-x86_64/bin/x86_64-linux-android34-
if [ ! -e ${WORKSPACE}/wolfssl-install ]; then
    echo "=== Installing WolfSSL ==="
    export WOLFSSL_CONFIG_OPTS='--enable-all-crypto --with-eccminsz=192 --with-max-ecc-bits=1024 --enable-opensslcoexist --enable-sha'
    export WOLFSSL_CONFIG_CPPFLAGS=CPPFLAGS="-I${WORKSPACE}/openssl-install -DWC_RSA_NO_PADDING -DWOLFSSL_PUBLIC_MP -DHAVE_PUBLIC_FFDHE -DHAVE_FFDHE_6144 -DHAVE_FFDHE_8192 -DWOLFSSL_PSS_LONG_SALT -DWOLFSSL_PSS_SALT_LEN_DISCOVER"
    if [ "${USE_FIPS}" = "true" ]; then
        WOLFSSL_CONFIG_OPTS+=' --enable-fips=ready'
        if [ "${USE_FIPS_CHECK}" = "true" ]; then
            git clone https://github.com/wolfssl/wolfssl ${WORKSPACE}/wolfssl && \
                cd ${WORKSPACE}/wolfssl && ./fips-check.sh fips-ready keep && \
                mv ${WORKSPACE}/wolfssl/XXX-fips-test ${WORKSPACE}/wolfssl-source && \
                rm -rf ${WORKSPACE}/wolfssl && \
                cd ${WORKSPACE}/wolfssl-source && ./autogen.sh
            checkReturn $?
        else
            wget -O ${WORKSPACE}/wolfssl-fips.zip https://www.wolfssl.com/wolfssl-5.6.4-gplv3-fips-ready.zip && \
                cd ${WORKSPACE} && unzip wolfssl-fips.zip && \
                mv ${WORKSPACE}/wolfssl-5.6.4-gplv3-fips-ready ${WORKSPACE}/wolfssl-source && \
                rm ${WORKSPACE}/wolfssl-fips.zip
            checkReturn $?
        fi
    else
        WOLFSSL_CONFIG_OPTS+=' --enable-curve25519 --enable-curve448 --enable-ed25519 --enable-ed448'
        git clone https://github.com/wolfssl/wolfssl ${WORKSPACE}/wolfssl-source && \
            cd ${WORKSPACE}/wolfssl-source && ./autogen.sh
        checkReturn $?
    fi
    cd ${WORKSPACE}/wolfssl-source && \
        CC=x86_64-linux-android34-clang ./configure ${WOLFSSL_CONFIG_OPTS} "${WOLFSSL_CONFIG_CPPFLAGS}" -prefix=${WORKSPACE}/wolfssl-install --host=x86_64-linux-android --disable-asm CFLAGS=-fPIC && \
        make
    checkReturn $?
    if [ "${USE_FIPS}" = "true" ]; then
        adb push --sync src/.libs/libwolfssl.so ./wolfcrypt/test/.libs/testwolfcrypt /data/local/tmp/ && \
        NEWHASH=$(adb shell "LD_LIBRARY_PATH=/data/local/tmp /data/local/tmp/testwolfcrypt 2>&1 | sed -n 's/hash = \(.*\)/\1/p'") && \
        sed -i "s/^\".*\";/\"${NEWHASH}\";/" wolfcrypt/src/fips_test.c && \
        checkReturn $?
    fi
    make -j install
    checkReturn $?
fi
export LD_LIBRARY_PATH="${WORKSPACE}/wolfssl-install/lib:$LD_LIBRARY_PATH"
export LIBRARY_PATH="${WORKSPACE}/wolfssl-install/lib:$LIBRARY_PATH"

echo "=== Installing wolfProvider ==="

# If running in wolfProvider/IDE/Android, then 'ln -s ../../ wolfProvider'
if [ ! -e ${WORKSPACE}/wolfProvider ]; then
    git clone https://github.com/wolfssl/wolfProvider ${WORKSPACE}/wolfProvider
    checkReturn $?
fi
cd ${WORKSPACE}/wolfProvider && \
    ./autogen.sh && \
    CC=x86_64-linux-android34-clang ./configure --with-openssl=${WORKSPACE}/openssl-install --with-wolfssl=${WORKSPACE}/wolfssl-install --host=x86_64-linux-android CFLAGS="-lm -fPIC" --enable-debug && \
    make -j
checkReturn $?

${CROSS_COMPILE}clang ${WORKSPACE}/wolfProvider/examples/openssl_example.c -I ${WORKSPACE}/openssl-install/include/ -L ${WORKSPACE}/openssl-install/lib/ -lcrypto -o ${WORKSPACE}/wolfProvider/examples/openssl_example
checkReturn $?

exit 0
