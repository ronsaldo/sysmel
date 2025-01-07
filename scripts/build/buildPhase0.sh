#!/bin/sh
set -ex

. $(dirname $(readlink -f "$0"))/buildCommon.sh

$SYSBVMI package-sources/BootstrapPackages.sysmel package-sources/SysmelCompilerPackages.sysmel
##$CC -o $OUT_DIR/phase0 $OUT_DIR/phase0.o $OUT_DIR/sysmel-pal.o $BUILD_LD_FLAGS
