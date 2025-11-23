/* Input the week day name and ordinal from an argument
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
#include <stddef.h>
#include <stdint.h>

#include "cmdtmio.h"

/* Parse the leading part of the specified argument as the week day name
   and ordinal separated by a comma and set those values into *WEEKDAY
   and *WEEKDAY_ORDINAL, storing the pointer to a following character into
   *ENDPTR. Return 1 or 0 if a value is set or not.  */

int
sscanweekday (const char *argv,
              int *weekday, intmax_t *weekday_ordinal, char **endptr)
{
  const struct word_table weekday_table[] =
    {
      { "SUNDAY", 0 }, { "MONDAY", 1 }, { "TUESDAY", 2 }, { "WEDNESDAY", 3 },
      { "THURSDAY", 4 }, { "FRIDAY", 5 }, { "SATURDAY", 6 }, { NULL, -1 }
    };
  int day_number;
  int set_num = sscanword (argv, weekday_table, 3, &day_number, endptr);
  if (set_num > 0)
    {
      intmax_t day_ordinal = 0;
      if (**endptr == ','
          && sscannumimax (*endptr + 1, &day_ordinal, endptr) < 0)
        return -1;

      *weekday = day_number;
      *weekday_ordinal = day_ordinal;
    }

  return set_num;
}
