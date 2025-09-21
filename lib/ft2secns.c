/* Convert the file time to seconds and nanoseconds since Unix epoch in NTFS
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

#include <windows.h>
#include <stdbool.h>
#include <stdint.h>

#include "timeoverflow.h"
#include "wintm.h"

/* Convert the specified file time to seconds and 100 nanoseconds since Unix
   epoch. Return true if conversion is performed, otherwise, false.  */

bool
ft2secns(const FILETIME *ft, intmax_t *seconds, int *nsec)
{
  LARGE_INTEGER ft_val;
  intmax_t ft_seconds;

  ft_val.HighPart = ft->dwHighDateTime;
  ft_val.LowPart = ft->dwLowDateTime;

  ft_seconds = ft_val.QuadPart / FILETIME_FRAC_PRECISION
               - FILETIME_UNIXEPOCH_SECONDS;

  if (! timew_overflow (ft_seconds))
    {
      *seconds = ft_seconds;

      /* Set the value of 100 nanoseconds into *NSEC if not NULL. */
      if (nsec)
        *nsec = ft_val.QuadPart % FILETIME_FRAC_PRECISION;

      return true;
    }

  return false;
}
