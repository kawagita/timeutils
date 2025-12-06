/* Output relative parameters of time to standard output
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

#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>

#include "cmdtmio.h"

/* Output parameters of date and time included in *TM_PTRS with a leading
   space in order to standard output. If NO_NEWLINE is true, output
   the trailing newline. Return the number of output parameters.  */

int
printreltm (bool no_newline, const struct tm_ptrs *tm_ptrs)
{
  int out_num = 0;

  /* Output the relative date and time. */
  if (tm_ptrs->dates)
    {
      int i;
      for (i = 0; i < 3; i++)
        {
          printf (" %d", *tm_ptrs->dates[i]);
          out_num++;
        }

      /* Output the relative hour, minutes, and seconds. */
      if (tm_ptrs->times || tm_ptrs->rel_times)
        {
          for (i = 0; i < 3; i++)
            {
              if (tm_ptrs->times)
                printf (" %d", *tm_ptrs->times[i]);
              else
                printf (" %" PRIdMAX, *tm_ptrs->rel_times[i]);
              out_num++;
            }

          /* Output the relative value of nanoseconds less than a second. */
          if (tm_ptrs->ns)
            {
              printf (" %d", *tm_ptrs->ns);
              out_num++;
            }
        }

      if (!no_newline)
        fputc ('\n', stdout);
    }

  return out_num;
}
