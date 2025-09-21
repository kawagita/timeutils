/* Get the number of days in leap years
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

#include <limits.h>
#include <stdbool.h>

#include "adjusttm.h"
#include "intoverflow.h"

/* Return true if a year divided by DIV is included in TERM since YEAR  */

#define INCLUDE_DIV_YEAR(year,term,div) \
  (((year % div + (div - 1)) % div + (term)) >= div)

/* Calculate the number of leap days between the specified years. If TO_YEAR
   is greater than, equal to, or less than FROM_YEAR, return the positive
   zero, or negateive value.  */

int
leapdays (int from_year, int to_year)
{
  if (from_year ^ to_year)
    {
      int ldays = 0;
      int signval = 1;
      int ystart = from_year;
      int yterm;
      int delta;

      /* Calculate the term bertween two years including themselves. */
      if (INT_SUBTRACT_WRAPV (to_year, from_year, &yterm)
          || ! (yterm ^ INT_MAX | yterm ^ INT_MIN))
        return from_year > 0 ? INT_MAX : INT_MIN;

      if (yterm < 0)
        {
          signval = -1;
          ystart = to_year;
        }

      yterm += signval;

      /* Count the number of leap days in each 400 year. */
      delta = yterm / 400;
      if (delta)
        {
          ldays += delta * 97;
          yterm -= delta * 400;
        }

      /* Count the number of leap days in each 100 year. */
      delta = yterm / 100;
      if (delta)
        {
          /* Increment leap days for the leap year divided by 400
             if it's one of years divided by 100. */
          if (yterm && INCLUDE_DIV_YEAR (ystart, yterm * signval, 400))
            ldays += signval;

          ldays += delta * 24;
          yterm -= delta * 100;
        }

      /* Count the number of leap days in each 4 year. */
      delta = yterm / 4;
      if (delta)
        {
          ldays += delta;
          yterm -= delta * 4;
        }

      /* Increment leap days for a leap year included in the rest. */
      if (yterm && INCLUDE_DIV_YEAR (ystart, yterm * signval, 4))
        ldays += signval;

      return ldays;
    }

  return HAS_NOLEAPDAY (from_year) ? 0 : 1;
}
