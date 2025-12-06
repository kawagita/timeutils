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

#include "argempty.h"
#include "argmatch.h"
#include "argnum.h"

extern const char *arg_endptr;

/* Parse the leading part of the specified argument as the week day name
   and ordinal separated by a comma and set those values into *WEEKDAY
   and *WEEKDAY_ORDINAL, storing the pointer to a following character into
   *ENDPTR. Return the number of set values, otherwise, -1 if the week day
   ordinal is outside the range of 0 to INTMAX_MAX.  */

int
argweekday (const char *arg,
            int *weekday, intmax_t *weekday_ordinal, char **endptr)
{
  char *endp;
  int set_num = 0;

  *endptr = (char *)arg;

  const struct arg_table days_of_week[] =
    {
      { "SUNDAY", 0 }, { "MONDAY", 1 }, { "TUESDAY", 2 }, { "WEDNESDAY", 3 },
      { "THURSDAY", 4 }, { "FRIDAY", 5 }, { "SATURDAY", 6 }, { NULL, -1 }
    };
  int day_number;
  if (argmatch (arg, days_of_week, 3, &day_number, &endp))
    {
      if (! argempty (endp))
        {
          if (*endp != ',')
            return 0;

          arg = endp + 1;
          *endptr = (char *)arg;

          intmax_t day_ordinal;
          int ord_num = argnumimax (arg, &day_ordinal, &endp);
          if (ord_num < 0)
            return -1;
          else if (ord_num == 0 || ! argempty (endp))
            return 0;

          *weekday_ordinal = day_ordinal;
          set_num++;
        }

      *weekday = day_number;
      set_num++;
      *endptr = (char *)arg_endptr;
    }

  return set_num;
}
