#!/bin/sh
set -ex

. $(dirname $(readlink -f "$0"))/buildCommon.sh

$CC -pthread -Wall -Wextra -Iinclude -I. -DNDEBUG $BUILD_CFLAGS -g -c -o$OUT_DIR/sysmel-pal.o -O2 -ffunction-sections -fdata-sections lib/support/support.c
##$CC -m32 -pthread -Wall -Wextra -Iinclude -I. -DNDEBUG -g -c -o$OUT_DIR32/sysmel-pal.o -O2 -ffunction-sections -fdata-sections lib/support/support.c
