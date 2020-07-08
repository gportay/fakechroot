/*
    libfakechroot -- fake chroot environment
    Copyright (c) 2020 Gaël PORTAY <gael.portay@gmail.com>

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

#ifdef HAVE_FCHDIR

#include <string.h>
#include "libfakechroot.h"
#include "getcwd_real.h"


wrapper(fchdir, int, (int fd))
{
    char cwd[FAKECHROOT_PATH_MAX];

    const char *fakechroot_base = getenv("FAKECHROOT_BASE");

    debug("fchdir(%d)", fd);
    int ret = nextcall(fchdir)(fd);
    if (ret) {
	    return ret;
    }

    if (getcwd_real(cwd, FAKECHROOT_PATH_MAX) == NULL) {
        return -1;
    }
    if (fakechroot_base != NULL && strstr(cwd, fakechroot_base) == NULL) {
        char *ld_library_path;
        unsetenv("FAKECHROOT_BASE");
        unsetenv("LD_LIBRARY_PATH");
        ld_library_path = getenv("FAKECHROOT_LDLIBPATH");
        if (ld_library_path)
            setenv("LD_LIBRARY_PATH", ld_library_path, 1);
    }

    return ret;
}

#else
typedef int empty_translation_unit;
#endif
