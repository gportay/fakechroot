#!/bin/sh

# ld-linux-x86-64.so.2.
#
# Replacement for /lib/ld-linux-x86-64.so.2. command which calls the program
# from the chroot'ed environment.
#
# (c) 2020 Gaël PORTAY <gael.portay@gmail.com>, LGPL

FAKECHROOT_BASE="$FAKECHROOT_BASE_ORIG" \
LD_LIBRARY_PATH="$FAKECHROOT_BASE_ORIG/usr/lib/x86_64-linux:$FAKECHROOT_BASE_ORIG/lib" \
LD_PRELOAD="$FAKECHROOT_BASE_ORIG/usr/lib/libfakeroot/fakechroot/libfakechroot.so:$LD_PRELOAD" \
/lib/x86_64-linux-gnu/ld-2.28.so \
"$FAKECHROOT_BASE_ORIG$FAKECHROOT_CMD_ORIG" "$@"
