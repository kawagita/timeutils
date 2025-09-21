/* Check whether a second overflows for time_T and in NTFS
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

/* Maximum and minimum seconds since Unix epoch, represented by file time  */
#define MAX_SECONDS_SINCE_UNIXEPOCH  910692730085LL
#define MIN_SECONDS_SINCE_UNIXEPOCH -933981677285LL

/* Maximum and minimum value for time_t  */
#define TIME_T_MAX \
  ((((intmax_t) 1 << (sizeof (time_t) * CHAR_BIT - 2)) - 1) * 2 + 1)

static intmax_t const time_t_max = (intmax_t)TIME_T_MAX;
static intmax_t const time_t_min = (intmax_t) ~ TIME_T_MAX;

/* Return true if the specified seconds is outside the range for time_t
   and represented by file time.  */

bool
timew_overflow (intmax_t seconds)
{
  if (MAX_SECONDS_SINCE_UNIXEPOCH < time_t_max
      ? seconds > MAX_SECONDS_SINCE_UNIXEPOCH : seconds > time_t_max)
    return true;
  else if (MIN_SECONDS_SINCE_UNIXEPOCH > time_t_min
      ? seconds < MIN_SECONDS_SINCE_UNIXEPOCH : seconds < time_t_min)
    return true;

  return false;
}
