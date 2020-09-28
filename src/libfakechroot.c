/*
    libfakechroot -- fake chroot environment
    Copyright (c) 2003-2015 Piotr Roszatycki <dexter@debian.org>
    Copyright (c) 2007 Mark Eichin <eichin@metacarta.com>
    Copyright (c) 2006, 2007 Alexander Shishkin <virtuoso@slind.org>

    klik2 support -- give direct access to a list of directories
    Copyright (c) 2006, 2007 Lionel Tricon <lionel.tricon@free.fr>

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

#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <pwd.h>
#include <dlfcn.h>

#include "setenv.h"
#include "libfakechroot.h"
#include "getcwd_real.h"
#include "strchrnul.h"

#define EXCLUDE_LIST_SIZE 100


/* Useful to exclude a list of directories or files */
static char *exclude_list[EXCLUDE_LIST_SIZE];
static int exclude_length[EXCLUDE_LIST_SIZE];
static int exclude_max = 0;
static char *exclude_symlink_list[EXCLUDE_LIST_SIZE];
static int exclude_symlink_length[EXCLUDE_LIST_SIZE];
static int exclude_symlink_max = 0;
static char *include_list[EXCLUDE_LIST_SIZE];
static int include_length[EXCLUDE_LIST_SIZE];
static int include_max = 0;
static int first = 0;


/* List of environment variables to preserve on clearenv() */
char *preserve_env_list[] = {
    "FAKECHROOT_BASE",
    "FAKECHROOT_CMD_SUBST",
    "FAKECHROOT_DEBUG",
    "FAKECHROOT_DETECT",
    "FAKECHROOT_ELFLOADER",
    "FAKECHROOT_ELFLOADER_OPT_ARGV0",
    "FAKECHROOT_INCLUDE_PATH",
    "FAKECHROOT_EXCLUDE_PATH",
    "FAKECHROOT_EXCLUDE_SYMLINK",
    "FAKECHROOT_LDLIBPATH",
    "FAKECHROOT_VERSION",
    "FAKEROOTKEY",
    "FAKED_MODE",
    "LD_LIBRARY_PATH",
    "LD_PRELOAD"
};
const int preserve_env_list_count = sizeof preserve_env_list / sizeof preserve_env_list[0];


LOCAL int fakechroot_debug (const char *fmt, ...)
{
    int ret;
    char newfmt[2048];

    va_list ap;
    va_start(ap, fmt);

    if (!getenv("FAKECHROOT_DEBUG"))
        return 0;

    sprintf(newfmt, PACKAGE ": %s\n", fmt);

    ret = vfprintf(stderr, newfmt, ap);
    va_end(ap);

    return ret;
}

#define list_init(path, list, length, list_max) \
    { \
        int i; \
            for (i = 0; list_max < EXCLUDE_LIST_SIZE; ) { \
                int j; \
                for (j = i; path[j] != ':' && path[j] != '\0'; j++); \
                list[list_max] = malloc(j - i + 2); \
                memset(list[list_max], '\0', j - i + 2); \
                strncpy(list[list_max], &(path[i]), j - i); \
                length[list_max] = strlen(list[list_max]); \
                list_max++; \
                if (path[j] != ':') break; \
                i = j + 1; \
            } \
    }

#include "getcwd.h"


/* Bootstrap the library */
void fakechroot_init (void) CONSTRUCTOR;
void fakechroot_init (void)
{
    char *detect = getenv("FAKECHROOT_DETECT");


    if (detect) {
        /* printf causes coredump on FreeBSD */
        if (write(STDOUT_FILENO, PACKAGE, sizeof(PACKAGE)-1) &&
            write(STDOUT_FILENO, " ", 1) &&
            write(STDOUT_FILENO, VERSION, sizeof(VERSION)-1) &&
            write(STDOUT_FILENO, "\n", 1)) { /* -Wunused-result */ }
        _Exit(atoi(detect));
    }

    debug("fakechroot_init()");
    debug("FAKECHROOT_BASE=\"%s\"", getenv("FAKECHROOT_BASE"));
    debug("FAKECHROOT_BASE_ORIG=\"%s\"", getenv("FAKECHROOT_BASE_ORIG"));
    debug("FAKECHROOT_CMD_ORIG=\"%s\"", getenv("FAKECHROOT_CMD_ORIG"));

    if (!first) {
        first = 1;

        /* We get a list of directories or files to exclude */
        char *list = getenv("FAKECHROOT_EXCLUDE_PATH");
        if (list)
            list_init(list, exclude_list, exclude_length, exclude_max);

        /* We get a list of directories of files to include */
        list = getenv("FAKECHROOT_INCLUDE_PATH");
        if (list)
            list_init(list, include_list, include_length, include_max);

        /* We get a list of symlinks to exclude */
        list = getenv("FAKECHROOT_EXCLUDE_SYMLINK");
        if (list)
            list_init(list, exclude_symlink_list, exclude_symlink_length, exclude_symlink_max);

        __setenv("FAKECHROOT", "true", 1);
        __setenv("FAKECHROOT_VERSION", FAKECHROOT, 1);
    }
}


/* Lazily load function */
LOCAL fakechroot_wrapperfn_t fakechroot_loadfunc (struct fakechroot_wrapper * w)
{
    char *msg;
    if (!(w->nextfunc = dlsym(RTLD_NEXT, w->name))) {;
        msg = dlerror();
        fprintf(stderr, "%s: %s: %s\n", PACKAGE, w->name, msg != NULL ? msg : "unresolved symbol");
        exit(EXIT_FAILURE);
    }
    return w->nextfunc;
}


/* Check if path is on list */
LOCAL int fakechroot_is_listed (const char * v_path, char * const list[], int length[], int list_max)
{
    const size_t len = strlen(v_path);
    int i;

    for (i = 0; i < list_max; i++) {
        if (length[i] > len ||
            v_path[length[i] - 1] != (list[i])[length[i] - 1] ||
            strncmp(list[i], v_path, length[i]) != 0) continue;
        if (length[i] == len || v_path[length[i]] == '/') return 1;
    }

    return 0;
}


/* Check if path is on exclude list */
LOCAL int fakechroot_localdir (const char * p_path)
{
    char *v_path = (char *)p_path;
    char cwd_path[FAKECHROOT_PATH_MAX];

    if (!p_path)
        return 0;

    if (!first)
        fakechroot_init();

    /* We need to expand relative paths */
    if (p_path[0] != '/') {
        getcwd_real(cwd_path, FAKECHROOT_PATH_MAX);
        v_path = cwd_path;
        narrow_chroot_path(v_path);
    }

    /* We try to find if we need direct access to a file */
    if (fakechroot_is_listed (v_path, include_list, include_length, include_max))
        return 1;

    return fakechroot_is_listed (v_path, exclude_list, exclude_length, exclude_max);
}


/* Check if symlink is on exclude list */
LOCAL int fakechroot_localsymlink (const char * p_path)
{
    char *v_path = (char *)p_path;
    char cwd_path[FAKECHROOT_PATH_MAX];

    if (!p_path)
        return 0;

    if (!first)
        fakechroot_init();

    /* We need to expand relative paths */
    if (p_path[0] != '/') {
        getcwd_real(cwd_path, FAKECHROOT_PATH_MAX);
        v_path = cwd_path;
        narrow_chroot_path(v_path);
    }

    /* We try to find if we need direct access to a file */
    return fakechroot_is_listed (v_path, exclude_symlink_list, exclude_symlink_length, exclude_symlink_max);
}


/*
 * Parse the FAKECHROOT_CMD_SUBST environment variable (the first
 * parameter) and if there is a match with filename, return the
 * substitution in cmd_subst.  Returns non-zero if there was a match.
 *
 * FAKECHROOT_CMD_SUBST=cmd=subst:cmd=subst:...
 */
LOCAL int fakechroot_try_cmd_subst (char * env, const char * filename, char * cmd_subst)
{
    int len, len2;
    char *p;

    if (env == NULL || filename == NULL)
        return 0;

    /* Remove trailing dot from filename */
    if (filename[0] == '.' && filename[1] == '/')
        filename++;
    len = strlen(filename);

    do {
        p = strchrnul(env, ':');

        if (strncmp(env, filename, len) == 0 && env[len] == '=') {
            len2 = p - &env[len+1];
            if (len2 >= FAKECHROOT_PATH_MAX)
                len2 = FAKECHROOT_PATH_MAX - 1;
            strncpy(cmd_subst, &env[len+1], len2);
            cmd_subst[len2] = '\0';
            return 1;
        }

        env = p;
    } while (*env++ != '\0');

    return 0;
}
