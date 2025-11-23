/* Convert file time in NTFS to seconds and nanoseconds since Unix epoch
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
#include "imaxoverflow.h"
#include "timeoverflow.h"

/* Convert the specified file time to seconds since 1970-01-01 00:00 UTC
   and 100 nanoseconds less than a second. Set its two values into *SECONDS
   and *NSEC and return true if conversion is performed, otherwise, return
   false.  */

bool
ft2sec (const FT *ft, intmax_t *seconds, int *nsec)
{
  intmax_t ft_seconds;
  int ft_nsec;

#ifdef USE_TM_GLIBC
  ft_seconds = ft->tv_sec;
  ft_nsec = ft->tv_nsec / 100;
#else
  LARGE_INTEGER ft_large = (LARGE_INTEGER) { .HighPart = ft->dwHighDateTime,
                                             .LowPart = ft->dwLowDateTime };
  intmax_t ft_val;

  if (IMAX_SUBTRACT_WRAPV (ft_large.QuadPart, FT_UNIXEPOCH_NSEC, &ft_val))
    return false;

  ft_seconds = ft_val / FT_NSEC_PRECISION;
  ft_nsec = ft_val % FT_NSEC_PRECISION;

  /* If nanoseconds is negative, add 1 to its value and decrement seconds,
     because tv_nsec is always a positive offset even if tv_sec is negative
     in the timespec convention and this program is corresponding to it. */
  if (ft_val < 0)
    {
      if (IMAX_SUBTRACT_WRAPV (ft_seconds, 1, &ft_seconds))
        return false;

      ft_nsec += FT_NSEC_PRECISION;
    }
#endif

  if (! timew_overflow (ft_seconds))
    {
      *seconds = ft_seconds;
      *nsec = ft_nsec;

      return true;
    }

  return false;
}
