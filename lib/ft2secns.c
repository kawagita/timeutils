/* Convert file time to seconds and nanoseconds since Unix epoch in NTFS
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

#include "ft.h"
#include "timeoverflow.h"

/* Convert the specified file time to seconds and 100 nanoseconds since Unix
   epoch. Set its value into *SECONDS but nanoseconds into *NSEC unless not
   NULL. Return true if conversion is performed, otherwise, false.  */

bool
ft2secns (const FT *ft, intmax_t *seconds, int *nsec)
{
  intmax_t ft_seconds;

#ifdef USE_TM_CYGWIN
  ft_seconds = ft->tv_sec;
#else
  LARGE_INTEGER ft_val;

  ft_val.HighPart = ft->dwHighDateTime;
  ft_val.LowPart = ft->dwLowDateTime;

  ft_seconds = ft_val.QuadPart / FT_FRAC_PRECISION - FT_UNIXEPOCH_SECONDS;
#endif

  if (! timew_overflow (ft_seconds))
    {
      *seconds = ft_seconds;

      /* Set the value of 100 nanoseconds into *NSEC if not NULL. */
      if (nsec)
        {
          long ft_nsec;

#ifdef USE_TM_CYGWIN
          ft_nsec = ft->tv_nsec;

          while (ft_nsec >= FT_FRAC_PRECISION)
            ft_nsec /= 10;
#else
          ft_nsec = ft_val.QuadPart % FT_FRAC_PRECISION;
#endif
          if (ft_nsec < 0)
            ft_nsec = - ft_nsec;

          *nsec = ft_nsec;
        }

      return true;
    }

  return false;
}
