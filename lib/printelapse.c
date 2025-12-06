/* Output parameters of time to standard output
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

#include "ftsec.h"

/* Output the specified seconds or nanoseconds elapsed since a time to
   standard output. If FLAC_VAL is not less than 0 or NO_NEWLINE is true,
   output its fractional value or the trailing newline. Return 1.  */

int
printelapse (bool no_newline, intmax_t elapse, int frac_val)
{
  int out_num = 0;

  /* Adjust seconds, which is that fractional part is always
     a positive value even if seconds is negative. */
  if (elapse < 0 && frac_val > 0)
    {
      /* Output the minus sign for -0.nnnnnnn. */
      if (++elapse == 0)
        fputc ('-', stdout);

      frac_val = FT_NSEC_PRECISION - frac_val;
    }

  printf ("%" PRIdMAX, elapse);
  out_num++;

  if (frac_val >= 0)
    printf (FT_NSEC_FORMAT, frac_val);

  if (!no_newline)
    fputc ('\n', stdout);

  return out_num;
}
