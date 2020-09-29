/*
    libfakechroot -- fake chroot environment
    Copyright (c) 2010, 2013, 2015 Piotr Roszatycki <dexter@debian.org>

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

#define _GNU_SOURCE
#include <errno.h>
#include <stddef.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include "setenv.h"
#include "libfakechroot.h"

#include "strlcpy.h"
#include "dedotdot.h"

#ifdef HAVE___XSTAT64
# include "__xstat64.h"
# define STAT_T stat64
# ifndef _STAT_VER
#  define _STAT_VER      3
# endif
# define STAT(path, sb) nextcall(__xstat64)(_STAT_VER, path, sb)
#else
# include "stat.h"
# define STAT_T stat
# define STAT(path, sb) nextcall(stat)(path, sb)
#endif

#include "getcwd_real.h"

#include <stdio.h>


wrapper(chroot, int, (const char * path))
{
    char fakechroot_abspath[FAKECHROOT_PATH_MAX];
    char fakechroot_buf[FAKECHROOT_PATH_MAX];

    char *ld_library_path, *separator, *new_ld_library_path;
    int status;
    size_t len;
    char cwd[FAKECHROOT_PATH_MAX - 1];
    char tmp[FAKECHROOT_PATH_MAX], *tmpptr = tmp;
    struct STAT_T sb;

    const char *fakechroot_base = getenv("FAKECHROOT_BASE");

    debug("chroot(\"%s\")", path);
    fprintf(stderr, "chroot(\"%s\")\n", path);

    if (!path) {
        __set_errno(EFAULT);
        return -1;
    }

    if (!*path) {
        __set_errno(ENOENT);
        return -1;
    }

    if (getcwd_real(cwd, FAKECHROOT_PATH_MAX - 1) == NULL) {
        __set_errno(EIO);
        return -1;
    }

#if 0
    if (strcmp(path, ".") == 0) {
        unsetenv("FAKECHROOT_BASE");
        unsetenv("LD_LIBRARY_PATH");
        ld_library_path = getenv("FAKECHROOT_LDLIBPATH");
        if (ld_library_path) {
            setenv("LD_LIBRARY_PATH", ld_library_path, 1);
        }
        chdir(fakechroot_base);
        return 0;
    }
#endif

    fprintf(stderr, "path: '%s'\n", path);
    fprintf(stderr, "cwd: '%s'\n", cwd);
    fprintf(stderr, "fakechroot_base: '%s'\n", fakechroot_base);
    if (fakechroot_base != NULL && strstr(cwd, fakechroot_base) == cwd) {
        expand_chroot_path(path);
        fprintf(stderr, "expand_chroot_path(): path: '%s'\n", path);
        strlcpy(tmp, path, FAKECHROOT_PATH_MAX);
        dedotdot(tmpptr);
        path = tmpptr;
        fprintf(stderr, "dedotdot(): path: '%s'\n", path);
    }
    else {
        size_t tmplen;
        if (*path == '/') {
            expand_chroot_rel_path(path);
            fprintf(stderr, "expand_chroot_rel_path(): path: '%s'\n", path);
            strlcpy(tmp, path, FAKECHROOT_PATH_MAX);
            dedotdot(tmpptr);
            path = tmpptr;
            fprintf(stderr, "dedotdot(): path: '%s'\n", path);
        }
        else {
            snprintf(tmp, FAKECHROOT_PATH_MAX, "%s/%s", cwd, path);
            dedotdot(tmpptr);
            path = tmpptr;
            fprintf(stderr, "dedotdot(): path: '%s'\n", path);
        }
        tmplen = strlen(tmpptr);
        while(tmplen > 1 && tmpptr[tmplen - 1] == '/') {
            tmpptr[--tmplen] = '\0';
        }
        fprintf(stderr, "suppress_trailing_slashes(): path: '%s'\n", path);
    }
    fprintf(stderr, "~ path: '%s'\n", path);

    /* Suppress a trailing slash */
// while(tmplen > 1 && tmpptr[tmplen - 1] == '/') {
// tmpptr[--tmplen] = '\0';
// }
    if ((strlen(tmpptr) > 1) && path[strlen(tmpptr) - 1] == '/') {
        tmpptr[strlen(tmpptr) - 1] = '\0';
    }
    fprintf(stderr, "suppress_trailing_slash(): path: '%s'\n", path);
    fprintf(stderr, "= path: '%s'\n", path);

    if ((status = STAT(path, &sb)) != 0) {
        return status;
    }

    if ((sb.st_mode & S_IFMT) != S_IFDIR) {
        __set_errno(ENOTDIR);
        return -1;
    }

    if (setenv("FAKECHROOT_BASE", path, 1) == -1) {
        return -1;
    }

    ld_library_path = getenv("LD_LIBRARY_PATH");

    if (ld_library_path != NULL && strlen(ld_library_path) > 0) {
        separator = ":";
    }
    else {
        ld_library_path = "";
        separator = "";
    }

    len = strlen(ld_library_path)+strlen(separator)+strlen(path)*2+sizeof("/usr/lib:/lib");

    if ((new_ld_library_path = malloc(len)) == NULL) {
        __set_errno(ENOMEM);
        return -1;
    }

    snprintf(new_ld_library_path, len, "%s%s%s/usr/lib:%s/lib", ld_library_path, separator, path, path);
    setenv("LD_LIBRARY_PATH", new_ld_library_path, 1);
    fprintf(stderr, "= new_ld_library_path: '%s'\n", new_ld_library_path);
    free(new_ld_library_path);

    return 0;
}
