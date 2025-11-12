/* Get the value of 100 nanoseconds since 1601-01-01 00:00 UTC from file time
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

#ifdef USE_TM_CYGWIN
# include <time.h>
#else
# include <windows.h>
#endif
#include <stdbool.h>
#include <stdint.h>

#include "ft.h"
#include "modifysec.h"

extern intmax_t sec2ftval (intmax_t seconds, int nsec, FT *ft);

/* Return the value of 100 nanoseconds since 1601-01-01 00:00 UTC converted
   from the specified file time, according to SEC_MODFLAG. If conversion is
   not performed, return 0.  */

intmax_t
toftval (const FT *ft, int sec_modflag)
{
  intmax_t seconds;
  int nsec;

# ifdef USE_TM_CYGWIN
  seconds = ft->tv_sec;
  nsec = ft->tv_nsec / 100;
# else
  if (!sec_modflag)
    {
      LARGE_INTEGER ft_large;

      ft_large.HighPart = ft->dwHighDateTime;
      ft_large.LowPart = ft->dwLowDateTime;

      return ft_large.QuadPart;
    }
  else if (! ft2sec (ft, &seconds, &nsec))
    return 0;
# endif

  if (! modifysec (&seconds, &nsec, sec_modflag))
    return 0;

  return sec2ftval (seconds, nsec, NULL);
}
