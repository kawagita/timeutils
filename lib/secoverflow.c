/* Check whether the value of seconds overflow in time_t and file time
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

#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>

#include "ftsec.h"
#include "ftval.h"

/* Maximum and minimum seconds since 1970-01-01 00:00 UTC, represented
   by FILETIME in Windows  */
static const intmax_t seconds_max =
  MAX_SECOND_IN_FILETIME - FILETIME_UNIXEPOCH_VALUE / FILETIME_SECOND_VALUE;
static const intmax_t seconds_min =
  MIN_SECOND_IN_FILETIME - FILETIME_UNIXEPOCH_VALUE / FILETIME_SECOND_VALUE;

/* Maximum 100 nanoseconds less than a second in the maximum seconds  */
static const int nsec_max =
  (intmax_t)0x7fffffffffffffff % FILETIME_SECOND_VALUE;

/* Maximum and minimum value represented by time_t  */
#define TIME_T_MAX \
  ((((intmax_t) 1 << (sizeof (time_t) * CHAR_BIT - 2)) - 1) * 2 + 1)

static const intmax_t time_t_max = (intmax_t)   TIME_T_MAX;
static const intmax_t time_t_min = (intmax_t) ~ TIME_T_MAX;

/* Return true if the specified seconds and nanoseconds less than a second
   is outside the range of time_t value in file time.  */

bool
secoverflow (intmax_t seconds, int nsec)
{
  if (seconds >= 0)
    {
#if !defined _WIN32 && !defined __CYGWIN__
      nsec /= FT_NSEC_PRECISION / FILETIME_SECOND_VALUE;  /* for GNU/Linx */
#endif

      return time_t_max < seconds_max ? seconds > time_t_max
             : (seconds > seconds_max
                || (seconds == seconds_max && nsec > nsec_max));
    }

  seconds++;
  nsec = FT_NSEC_PRECISION - nsec;
#if !defined _WIN32 && !defined __CYGWIN__
  nsec /= FT_NSEC_PRECISION / FILETIME_SECOND_VALUE;  /* for GNU/Linx */
#endif

  return time_t_min > seconds_min ? seconds < time_t_min
         : (seconds < seconds_min
            || (seconds == seconds_min && nsec > nsec_max + 1));
}
