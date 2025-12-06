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
#include "ftval.h"

/* Convert the specified file time to the value of 100 nanoseconds since
   1601-01-01 00:00 UTC according to FT_MODFLAG. Set its value into *FT_VAL
   and return true if conversion is performed, otherwise, return false.  */

bool
ft2val (const FT *ft, int ft_modflag, intmax_t *ft_val)
{
  intmax_t sec;
  int ns;

#ifdef USE_TM_GLIBC
  sec = ft->tv_sec;
  ns = GET_FT_NSEC (ft);

  if (secoverflow (sec, ns)
      || (ft_modflag && ! modifysec (&sec, &ns, ft_modflag)))
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
  else if (! ft2sec (ft, &sec, &ns) || ! modifysec (&sec, &ns, ft_modflag))
    return false;
#endif

  /* Increment seconds and subtract its value from nanoseconds because
     tv_nsec is always a positive offset even if tv_sec is negative in
     the timespec convention and this program is corresponding to it. */
  if (sec < 0 && ns > 0)
    {
      sec++;
      ns -= FT_NSEC_PRECISION;
    }

#if !defined _WIN32 && !defined __CYGWIN__
  ns /= FT_NSEC_PRECISION / FILETIME_SECOND_VALUE;  /* for GNU/Linx */
#endif

  *ft_val = sec * FILETIME_SECOND_VALUE + ns + FILETIME_UNIXEPOCH_VALUE;

  return true;
}
