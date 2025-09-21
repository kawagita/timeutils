/* Convert seconds and nanoseconds since Unix epoch to the file time in NTFS
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

/* Convert the specified seconds and nanoseconds since Unix epoch to file
   time and set its value into *FT. Return true if conversion is performed,
   otherwise, false.  */

bool
secns2ft (const intmax_t seconds, const int nsec, FILETIME *ft)
{
  if (! timew_overflow (seconds))
    {
      LARGE_INTEGER ft_val;

      ft_val.QuadPart = ((LONGLONG)seconds + FILETIME_UNIXEPOCH_SECONDS)
                        * FILETIME_FRAC_PRECISION;

      if (nsec >= 0 && nsec < FILETIME_FRAC_PRECISION)
        {
          ft_val.QuadPart += nsec;
          ft->dwHighDateTime = ft_val.HighPart;
          ft->dwLowDateTime = ft_val.LowPart;

          return true;
        }
    }

  return false;
}
