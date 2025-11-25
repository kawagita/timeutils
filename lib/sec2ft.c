/* Convert seconds and nanoseconds since Unix epoch to file time in NTFS
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

#ifdef USE_TM_GLIBC
# include <time.h>
#else
# include <windows.h>
#endif
#include <stdbool.h>
#include <stdint.h>

#include "ft.h"
#include "ftsec.h"

/* Convert the specified seconds since 1970-01-01 00:00 UTC and 100
   nanoseconds less than a second to file time. Set its value into *FT
   and return true if NSEC is not less than 0 and conversion is performed,
   otherwise, return false.  */

bool
sec2ft (intmax_t seconds, int nsec, FT *ft)
{
  if (nsec >= 0 && ! secoverflow (seconds, nsec))
    {
#ifdef USE_TM_GLIBC
      ft->tv_sec = seconds;
      ft->tv_nsec = nsec * 100;

      return true;
#else
      /* Subtract 1 from nanoseconds and increment seconds because tv_nsec
         is always a positive offset even if tv_sec is negative in
         the timespec convention and this program is corresponding to it. */
      intmax_t ft_seconds = seconds;
      int ft_nsec = nsec;
      if (ft_seconds < 0 && ft_nsec > 0)
        {
          ft_seconds++;
          ft_nsec -= FT_NSEC_PRECISION;
        }

      LARGE_INTEGER ft_large =
        (LARGE_INTEGER) { .QuadPart = ft_seconds * FT_NSEC_PRECISION
                                      + ft_nsec + FT_UNIXEPOCH_VALUE };

      ft->dwHighDateTime = ft_large.HighPart;
      ft->dwLowDateTime = ft_large.LowPart;

      return true;
#endif
    }

  return false;
}
