#!/bin/sh

# unshare
#
# Replacement for busybox command which calls the program from the chroot
# environment.  It sets the LD
#
# (c) 2020 Gaël PORTAY <gael.portay@gmail.com>, LGPL

echo "$0" "$@"
export
echo FAKECHROOT_BASE_ORIG=$FAKECHROOT_BASE_ORIG
echo FAKECHROOT_BASE=$FAKECHROOT_BASE
echo FAKECHROOT_CMD_ORIG=$FAKECHROOT_CMD_ORIG
echo FAKECHROOT_CMD=$FAKECHROOT_CMD
exit 0
$PWD/lib/ld-musl-x86_64.so.1 $PWD/bin/busybox
echo About to run $PWD/bin/busybox "$@"...
LD_PRELOAD=libfakechroot.so $PWD/lib/ld-musl-x86_64.so.1 -- $PWD/bin/busybox "$@"
echo Done
