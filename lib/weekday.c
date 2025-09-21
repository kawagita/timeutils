/* Get the week day in a year
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

#include <stdbool.h>

#include "adjusttm.h"

/* The week day at January 1st in Year 0  */
#define YEAR0_1ST_WEEKDAY 6

/* Return the week day for the specified day in YEAR.  */

int
weekday (int year, int yday)
{
  int days = yday % 7 + (yday < 0 ? 7 : 0);
  int y = year % 400;
  if (y)
    {
      if (y < 0)
        y += 400;
      adjusttm (&days, DAYS_IN_100YEARS, &y, 100);
      adjusttm (&days, DAYS_IN_4YEARS, &y, 4);
      days += DAYS_IN_YEAR * y;

      /* Don't increment the number of days for a leap day in YEAR
         but increment for the leap year divided by 400 because it's
         counted for years from 0 to y - 1. */
      if (HAS_NOLEAPDAY (y))
        days++;
    }

  return (days + YEAR0_1ST_WEEKDAY) % 7;
}
