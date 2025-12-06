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

#include "argempty.h"
#include "argnum.h"
#include "cmdtmio.h"
#include "ftsec.h"

/* Parse the specified arguments as the relative year, month, day, hour,
   minutes, seconds, and nanoseconds in order and set those values into
   members included in *TM_PTRS, storing the pointer to an error argument
   into *ERRARG if not set. Return the number of set values, otherwise, -1
   if a value is outside the range of its parameter.  */

int
argreltm (int argc, const char **argv, struct tm_ptrs *tm_ptrs, char **errarg)
{
  int set_num = 0;

  /* Input the relative date and time. */
  if (tm_ptrs->dates)
    {
      struct numint_prop date_props[] =
        {
          { 0, INT_MIN + 1900, INT_MAX, 0, NULL },
          { 0, INT_MIN + 1, INT_MAX, 0, NULL },
          { 0, INT_MIN, INT_MAX, 0, NULL }
        };
      char *endp;
      int i;
      for (i = 0; i < 3; i++)
        {
          if (--argc < 0)
            return set_num;

          int date;
          int date_num;
          if (tm_ptrs->rel_times)
            date_num = argnumint (*argv, &date, &endp);
          else
            date_num = argnumintp (*argv, date_props + i, &date, &endp);
          if (date_num <= 0 || ! argempty (endp))
            {
              *errarg = (char *)*argv;

              if (date_num < 0)
                return -1;

              return set_num;
            }

          *tm_ptrs->dates[i] = date;
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

              intmax_t rel_time;
              int time;
              int time_num;
              if (tm_ptrs->times)
                time_num = argnumint (*argv, &time, &endp);
              else
                time_num = argnumimax (*argv, &rel_time, &endp);
              if (time_num <= 0 || ! argempty (endp))
                {
                  *errarg = (char *)*argv;

                  if (time_num < 0)
                    return -1;

                  return set_num;
                }

              if (tm_ptrs->times)
                *tm_ptrs->times[i] = time;
              else
                *tm_ptrs->rel_times[i] = rel_time;
              set_num++;
              argv++;
            }

          /* Input the relative value of nanoseconds less than a second. */
          if (tm_ptrs->ns)
            {
              if (--argc < 0)
                return set_num;

              struct numint_prop ns_prop =
                { 0, 1 - FT_NSEC_PRECISION, FT_NSEC_PRECISION - 1, 0, NULL };
              int ns;
              int ns_num = argnumintp (*argv, &ns_prop, &ns, &endp);
              if (ns_num <= 0 || ! argempty (endp))
                {
                  *errarg = (char *)*argv;

                  if (ns_num < 0)
                    return -1;

                  return set_num;
                }

              *tm_ptrs->ns = ns;
              set_num++;
            }
        }
    }

  return set_num;
}
