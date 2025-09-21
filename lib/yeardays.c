/* Get the number of days in a year
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

/* Days for months since January in a year  */

static const int ydays[] =
{
  0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, DAYS_IN_YEAR
};

/* Return the number of days for the specified months since January in
   a year if its value is from 0 to 12, otherwise, -1.  */

int
yeardays(bool has_noleapday, int months)
{
  if (months >= 0 && months <= 12)
    {
      if (has_noleapday || months < 2)
        return ydays[months];

      return ydays[months] + 1;
    }

  return -1;
}
