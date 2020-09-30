#!/bin/sh

# busybox
#
# Replacement for busybox command which calls the program from the chroot'ed
# environment.
#
# (c) 2020 Gaël PORTAY <gael.portay@gmail.com>, LGPL

# The script is called:
# /usr/bin/busybox.fakechroot --install -s
# export FAKECHROOT="true"
# export FAKECHROOT_BASE_ORIG="/path/to/chrooted/dir"
# export FAKECHROOT_CMD_ORIG="/bin/busybox"
# export FAKECHROOT_CMD_SUBST="/usr/sbin/mount=/bin/true:/usr/sbin/umount=/bin/true:/bin/busybox=/usr/bin/busybox.fakechroot:/usr/sbin/chroot=/usr/bin/chroot.fakechroot:/usr/bin/ldd=/usr/bin/ldd.fakechroot:"
# export FAKECHROOT_EXCLUDE_PATH="/dev:/proc:/sys:/bin/sh"
# export FAKECHROOT_INCLUDE_PATH="/etc/shadow"
# export FAKECHROOT_VERSION="2.20.2"
# export FAKED_MODE="unknown-is-root"
# export FAKEROOTKEY="123456789"
# export LD_LIBRARY_PATH="/usr/lib/libfakeroot/fakechroot:/usr/lib/libfakeroot:/usr/lib64/libfakeroot:/usr/lib32/libfakeroot:/path/to/chrooted/dir/usr/lib:/path/to/chrooted/dir/lib"
# export LD_PRELOAD="libfakechroot.so:libfakeroot.so"
# export OLDPWD
# export PATH="/usr/sbin:/usr/bin:/sbin:/bin"
# export PWD="/path/to/chrooted/dir"
# export SHLVL="2"

FAKECHROOT_BASE="$FAKECHROOT_BASE_ORIG" \
LD_LIBRARY_PATH="$FAKECHROOT_BASE_ORIG/usr/lib:$FAKECHROOT_BASE_ORIG/lib" \
LD_PRELOAD="$FAKECHROOT_BASE_ORIG/usr/lib/libfakeroot/fakechroot/libfakechroot.so:$LD_PRELOAD" \
/lib/ld-musl-x86_64.so.1 --argv0 "$FAKECHROOT_CMD_ORIG" \
"$FAKECHROOT_BASE_ORIG$FAKECHROOT_CMD_ORIG" "$@" \
