#!/bin/bash -xeu

which icc   && CC=icc
which gcc   && CC=gcc
which clang && CC=clang
CF='-O0 -g -Wall -Werror -fwrapv'

cat /proc/cpuinfo | grep 'avx' > /dev/null && CF="${CF} -mavx"

LF=''
LIB=''
case "${OSTYPE}" in 
  linux*)
    LF='-ldl ${LF}'
    LIB='-fPIC -shared ${LIB}'
  ;;
  darwin*)
    LIB='-dynamiclib ${LIB}'
  ;;
esac

case "${CC}" in
  gcc*)
    CC="${CC} -Wno-unused -Wno-format"
  ;;
  clang*)
    CF="${CF} -ffunction-sections -fdata-sections -Wno-null-dereference -Wno-unused-function -fsanitize-undefined-trap-on-error -Wno-unused-variable -fsanitize=null -Wno-c++11-compat-deprecated-writable-strings -fno-omit-frame-pointer -x c"
  ;;
esac

echo 'CC='${CC}

SRC_DIR=$(pwd)
BUILD_DIR='build'

test -d ${BUILD_DIR} || mkdir -p ${BUILD_DIR}
pushd ${BUILD_DIR} 

${CC} ${CF} ${SRC_DIR}/test_hl.c -o hl_test ${LF}
./hl_test

popd
