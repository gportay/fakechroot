#!/bin/sh

# busybox
#
# Replacement for busybox command which calls the program from the chroot
# environment.  It sets the LD
#
# (c) 2020 Gaël PORTAY <gael.portay@gmail.com>, LGPL

# The script is called:
# /usr/bin/busybox.fakechroot --install -s
# export FAKECHROOT="true"
# export FAKECHROOT_BASE_ORIG="/home/gportay/src/makeroot/rootfs-alpinelinux"
# export FAKECHROOT_CMD_ORIG="/bin/busybox"
# export FAKECHROOT_CMD_SUBST="/usr/sbin/mount=/bin/true:/usr/sbin/umount=/bin/true:/bin/busybox=/usr/bin/busybox.fakechroot:/usr/sbin/chroot=/usr/bin/chroot.fakechroot:/usr/bin/ldd=/usr/bin/ldd.fakechroot:"
# export FAKECHROOT_EXCLUDE_PATH="/dev:/proc:/sys:/bin/sh"
# export FAKECHROOT_INCLUDE_PATH="/etc/shadow"
# export FAKECHROOT_VERSION="2.20.2"
# export FAKED_MODE="unknown-is-root"
# export FAKEROOTKEY="548803544"
# export LD_LIBRARY_PATH="/usr/lib/libfakeroot/fakechroot:/usr/lib/libfakeroot:/usr/lib64/libfakeroot:/usr/lib32/libfakeroot:/home/gportay/src/makeroot/rootfs-alpinelinux/usr/lib:/home/gportay/src/makeroot/rootfs-alpinelinux/lib"
# export LD_PRELOAD="libfakechroot.so:libfakeroot.so"
# export OLDPWD
# export PATH="/usr/sbin:/usr/bin:/sbin:/bin"
# export PWD="/home/gportay/src/makeroot/rootfs-alpinelinux"
# export SHLVL="2"

echo "$0" "$@"
echo "$0 FAKECHROOT_BASE_ORIG=$FAKECHROOT_BASE_ORIG"
echo "$0 FAKECHROOT_BASE=$FAKECHROOT_BASE"
echo "$0 FAKECHROOT_CMD_ORIG=$FAKECHROOT_CMD_ORIG"
export

echo About to run \
"$PWD/lib/ld-musl-x86_64.so.1" \
"$PWD/$FAKECHROOT_CMD_ORIG" "$@" \
"$FAKECHROOT_BASE/bin"

if LD_LIBRARY_PATH= \
   LD_PRELOAD= \
   "$PWD/lib/ld-musl-x86_64.so.1" \
   "$PWD/$FAKECHROOT_CMD_ORIG" "$@" \
   "$FAKECHROOT_BASE_ORIG/bin"
then
	:
else
	ret=$?
	echo ^ >&2
	echo   >&2
	echo   >&2
	echo   >&2
	echo Oops, exited with $ret >&2
	exit $ret
fi
