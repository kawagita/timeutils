/* Input relative parameters of time from arguments
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

/* Parse the specified arguments as relative parameters of time included
   in *TM_PTRS and set those values into the member, storing the pointer
   to a following character into *ENDPTR. Return the number of set values,
   otherwise, -1 if a value is outside the range of its parameter.  */

int
sscanreltm (int argc, const char **argv,
            struct tm_ptrs *tm_ptrs, char **endptr)
{
  int set_num = 0;

  /* Input the relative date and time. */
  if (tm_ptrs->dates)
    {
      const struct numint_prop date_props[] =
        {
          { 0, INT_MIN + TM_YEAR_BASE, INT_MAX, false },
          { 0, INT_MIN + 1, INT_MAX, false },
          { 0, INT_MIN, INT_MAX, false }
        };
      int i;
      for (i = 0; i < 3; i++)
        {
          if (--argc < 0)
            return set_num;

          int date_num;
          if (tm_ptrs->rel_times)
            date_num = sscannumint (*argv, tm_ptrs->dates[i], endptr);
          else
            date_num = sscannumintp (*argv, date_props + i,
                                     tm_ptrs->dates[i], NULL, endptr);
          if (date_num < 0)
            return -1;
          else if (date_num == 0 || **endptr != '\0')
            return set_num;

          set_num++;
          argv++;
        }

      /* Intput the relative hour, minutes, and seconds. */
      if (tm_ptrs->times || tm_ptrs->rel_times)
        {
          for (i = 0; i < 3; i++)
            {
              if (--argc < 0)
                return set_num;

              int time_num;
              if (tm_ptrs->times)
                time_num = sscannumint (*argv, tm_ptrs->times[i], endptr);
              else
                time_num = sscannumimax (*argv, tm_ptrs->rel_times[i], endptr);
              if (time_num < 0)
                return -1;
              else if (time_num == 0 || **endptr != '\0')
                return set_num;

              set_num++;
              argv++;
            }

          /* Input the relative value of fractional part in seconds. */
          if (tm_ptrs->frac_val && --argc >= 0)
            {
              const struct numint_prop frac_prop =
                { 0, - TM_FRAC_MAX, TM_FRAC_MAX, false };
              int frac_num = sscannumintp (*argv, &frac_prop,
                                           tm_ptrs->frac_val, NULL, endptr);
              if (frac_num < 0)
                return -1;
              else if (frac_num == 0 || **endptr != '\0')
                return set_num;

              set_num++;
            }
        }
    }

  return set_num;
}
