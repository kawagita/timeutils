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

#include <windows.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef USE_TM_CYGWIN
# include <time.h>
#endif

#include "ft.h"
#include "timeoverflow.h"

/* Convert the specified seconds and 100 nanoseconds since Unix epoch
   to file time and set its value into *FT unless not NULL. Return 100
   nanoseconds since 1601-01-01 00:00 UTC of file time if NSEC is more
   than or equal to 0 and conversion is performed, otherwise, 0.  */

intmax_t
secns2ftval (intmax_t seconds, int nsec, FT *ft)
{
  if (nsec >= 0 && ! timew_overflow (seconds))
    {
      LARGE_INTEGER ft_val;

      while (nsec >= FT_FRAC_PRECISION)
        nsec /= 10;

      ft_val.QuadPart = ((LONGLONG)seconds + FT_UNIXEPOCH_SECONDS)
                        * FT_FRAC_PRECISION + nsec;

      if (ft)
        {
#ifdef USE_TM_CYGWIN
          ft->tv_sec = seconds;
          ft->tv_nsec = nsec;
#else
          ft->dwHighDateTime = ft_val.HighPart;
          ft->dwLowDateTime = ft_val.LowPart;
#endif
        }

      return ft_val.QuadPart;
    }

  return 0;
}

/* Convert the specified seconds and 100 nanoseconds since Unix epoch to
   file time and set its value into *FT. Return true if NSEC is more than
   or equal to 0 and conversion is performed, otherwise, false.  */

bool
secns2ft (intmax_t seconds, int nsec, FT *ft)
{
  if (seconds || nsec)
    return secns2ftval (seconds, nsec, ft) != 0L;

#ifdef USE_TM_CYGWIN
  ft->tv_sec = 0;
  ft->tv_nsec = 0L;
#else
  ft->dwHighDateTime = ft->dwLowDateTime = 0;
#endif

  return true;
}
