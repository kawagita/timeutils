/* Output the setting name of adjusting by DST offset to standard output
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
#include <stdio.h>
#include <stdint.h>

#include "cmdtmio.h"

/* Output the setting name whether the time is adjusted by DST offset
   in time zone for the specified flag with a leading space to standard
   output. If NO_NEWLINE is true, output the trailing newline. Return
   1 if the flag is not less than 0, otherwise, 0.  */

int
printisdst (bool no_newline, int isdst)
{
  int out_num = 0;

  if (isdst >= 0)
    {
      printf (" %s", isdst > 0 ? DST_NAME : ST_NAME);
      out_num++;

      if (!no_newline)
        fputc ('\n', stdout);
    }
  else if (!no_newline)
    fputc ('\n', stdout);

  return out_num;
}
