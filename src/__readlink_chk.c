/*
    libfakechroot -- fake chroot environment
    Copyright (c) 2010, 2013 Piotr Roszatycki <dexter@debian.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
*/


#include <config.h>

#ifdef HAVE___READLINK_CHK

#define _FORTIFY_SOURCE 2
#include <stddef.h>
#include <unistd.h>
#include "libfakechroot.h"


wrapper(__readlink_chk, ssize_t, (const char * path, char * buf, size_t bufsiz, size_t buflen))
{
    char fakechroot_abspath[FAKECHROOT_PATH_MAX];
    char fakechroot_buf[FAKECHROOT_PATH_MAX];

    int linksize;
    char tmp[FAKECHROOT_PATH_MAX], *tmpptr;

    const char *fakechroot_base = getenv("FAKECHROOT_BASE");

    debug("__readlink_chk(\"%s\", &buf, %zd, %zd)", path, bufsiz, buflen);
    expand_chroot_path(path);

    if ((linksize = nextcall(__readlink_chk)(path, tmp, FAKECHROOT_PATH_MAX-1, buflen)) == -1) {
        return -1;
    }
    tmp[linksize] = '\0';

    if (fakechroot_base != NULL) {
        tmpptr = strstr(tmp, fakechroot_base);
        if (tmpptr != tmp) {
            tmpptr = tmp;
        }
        else if (tmp[strlen(fakechroot_base)] == '\0') {
            tmpptr = "/";
            linksize = strlen(tmpptr);
        }
        else if (tmp[strlen(fakechroot_base)] == '/') {
            tmpptr = tmp + strlen(fakechroot_base);
            linksize -= strlen(fakechroot_base);
        }
        else {
            tmpptr = tmp;
        }
        if (strlen(tmpptr) > bufsiz) {
            linksize = bufsiz;
        }
        strncpy(buf, tmpptr, linksize);
    }
    else {
        strncpy(buf, tmp, linksize);
    }
    return linksize;
}

#else
typedef int empty_translation_unit;
#endif
