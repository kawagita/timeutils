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
#include "ftval.h"

/* Convert the specified seconds since 1970-01-01 00:00 UTC and nanoseconds
   less than a second to file time. Set its value into *FT and return true
   if NSEC isn't less than 0 and conversion is performed, otherwise, return
   false.  */

bool
sec2ft (intmax_t seconds, int nsec, FT *ft)
{
  if (nsec >= 0 && ! secoverflow (seconds, nsec))
    {
#ifdef USE_TM_GLIBC
      ft->tv_sec = seconds;
      SET_FT_NSEC (ft, nsec);

      return true;
#else
      intmax_t sec = seconds;
      int ns = nsec;

      /* Increment seconds and subtract its value from nanoseconds because
         tv_nsec is always a positive offset even if tv_sec is negative in
         the timespec convention and this program is corresponding to it. */
      if (sec < 0 && ns > 0)
        {
          sec++;
          ns -= FT_NSEC_PRECISION;
        }

      LARGE_INTEGER ft_large =
        (LARGE_INTEGER) { .QuadPart = sec * FILETIME_SECOND_VALUE
                                      + ns + FILETIME_UNIXEPOCH_VALUE };

      ft->dwHighDateTime = ft_large.HighPart;
      ft->dwLowDateTime = ft_large.LowPart;

      return true;
#endif
    }

  return false;
}
