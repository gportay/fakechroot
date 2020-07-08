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

#include "libfakechroot.h"
#include "getcwd_real.h"


wrapper(fchdir, int, (int fd))
{
    debug("fchdir(%d)", fd);
    return nextcall(fchdir)(fd);
}

#else
typedef int empty_translation_unit;
#endif
