/* Input parameters of time from an argument
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
#include <stddef.h>
#include <stdint.h>

#include "cmdtmio.h"

/* Parse the leading part of the specified argument as parameters of time
   included in *TM_PTRS and set those values into the member, storing
   the pointer to a following character into *ENDPTR. Return the number
   of set values, otherwise, -1 if a value is outside the range of its
   parameter.  */

int
sscantm (const char *argv, struct tm_ptrs *tm_ptrs, char **endptr)
{
  int set_num = 0;

  /* Input the date and time. */
  if (tm_ptrs->dates)
    {
      const struct numint_prop tmint_prop = { 1, 0, INT_MAX, false };
      const struct numint_prop date_props[] =
        {
          { 0, -1, INT_MAX, 0 }, tmint_prop, tmint_prop
        };
      const char *p = argv;
      int i;
      for (i = 0; i < 3; i++)
        {
          bool negative = false;

          if (i > 0)
            {
              if (*p == '+')
                negative = true;
              else if (*p != '-')
                return set_num;

              p++;
            }

          int date_num = sscannumintp (p, date_props + i,
                                       tm_ptrs->dates[i], NULL, endptr);
          if (date_num < 0)
            return -1;
          else if (date_num == 0)
            return set_num;
          else if (negative)
            *tm_ptrs->dates[i] = - *tm_ptrs->dates[i];

          set_num++;
          p = *endptr;
        }

      /* Intput the hour, minutes, and seconds. */
      if (tm_ptrs->times)
        {
          if (*p != 'T')
            return set_num;

          for (i = 0; i < 3; i++)
            {
              if (i > 0 && *p != ':')
                return set_num;

              int time_num = sscannumintp (++p, &tmint_prop,
                                           tm_ptrs->times[i], NULL, endptr);
              if (time_num < 0)
                return -1;
              else if (time_num == 0)
                return set_num;

              set_num++;
              p = *endptr;
            }

          /* Input the value of fractional part in seconds. */
          if (tm_ptrs->frac_val && (*p == '.' || *p == ','))
            {
              const struct numint_prop frac_prop = { 1, 0, TM_FRAC_MAX, true };
              int frac_num = sscannumintp (p + 1, &frac_prop,
                                           tm_ptrs->frac_val, NULL, endptr);
              if (frac_num < 0)
                return -1;
              else if (frac_num == 0)
                return set_num;

              set_num++;
              p = *endptr;
            }

          /* Intput the UTC offset in a time zone. */
          if (tm_ptrs->utcoff && (*p == '-' || *p == '+'))
            {
              const struct numint_prop utcoff_prop =
                { *p == '-' ? -1 : 1, 0, 2400, false };
              int hhmm_val = 0;
              int utcoff_num = sscannumintp (p, &utcoff_prop,
                                             &hhmm_val, NULL, endptr);
              if (utcoff_num < 0)
                return -1;
              else if (utcoff_num > 0)
                {
                  int utcoff = ((hhmm_val / 100) * 60 + hhmm_val % 100) * 60;
                  if (utcoff_prop.sign < 0)
                    utcoff = - utcoff;

                  *tm_ptrs->utcoff = utcoff;
                  set_num++;
                }
            }
        }
    }

  return set_num;
}
