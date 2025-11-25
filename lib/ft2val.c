/* Convert file time in NTFS to 100 nanoseconds since 1601-01-01 00:00 UTC
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

/* Convert the specified file time to the value of 100 nanoseconds since
   1601-01-01 00:00 UTC according to FT_MODFLAG. Set its value into *FT_VAL
   and return true if conversion is performed, otherwise, return false.  */

bool
ft2val (const FT *ft, int ft_modflag, intmax_t *ft_val)
{
  intmax_t ft_seconds;
  int ft_nsec;

#ifdef USE_TM_GLIBC
  ft_seconds = ft->tv_sec;
  ft_nsec = ft->tv_nsec / 100;

  if (secoverflow (ft_seconds, ft_nsec))
    return false;
#else
  if (!ft_modflag)
    {
      LARGE_INTEGER ft_large =
        (LARGE_INTEGER) { .HighPart = ft->dwHighDateTime,
                          .LowPart = ft->dwLowDateTime };

      *ft_val = ft_large.QuadPart;

      return true;
    }
  else if (! ft2sec (ft, &ft_seconds, &ft_nsec))
    return false;
#endif
  else if (! modifysec (&ft_seconds, &ft_nsec, ft_modflag))
    return false;

  /* Subtract 1 from nanoseconds and increment seconds because tv_nsec
     is always a positive offset even if tv_sec is negative in
     the timespec convention and this program is corresponding to it. */
  if (ft_seconds < 0 && ft_nsec > 0)
    {
      ft_seconds++;
      ft_nsec -= FT_NSEC_PRECISION;
    }

  *ft_val = ft_seconds * FT_NSEC_PRECISION + ft_nsec + FT_UNIXEPOCH_VALUE;

  return true;
}
