/* Get 100 nanoseconds since 1601-01-01 00:00 UTC of file time in NTFS
   Copyright (C) 2025 Yoshinori Kawagita.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.  */

#include "config.h"

#ifndef USE_TM_CYGWIN
# include <windows.h>
#endif
#include <stdbool.h>
#include <stdint.h>

#ifdef USE_TM_CYGWIN
# include <time.h>
#endif

#include "wintm.h"

extern intmax_t secns2ftval (intmax_t seconds, int nsec, FT *ft);

/* Return 100 nanoseconds since 1601-01-01 00:00 UTC converted from
   the specified file time. If conversion is not performed, return 0.  */

intmax_t
toftval (const FT *ft)
{
# ifdef USE_TM_CYGWIN
  return secns2ftval (ft->tv_sec, ft->tv_nsec, NULL);
# else
  LARGE_INTEGER ft_val;

  ft_val.HighPart = ft->dwHighDateTime;
  ft_val.LowPart = ft->dwLowDateTime;

  return ft_val.QuadPart;
# endif
}
