/* Input Unix seconds and fractional part from an argument
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

#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "cmdtmio.h"
#include "imaxoverflow.h"

/* Parse the leading part of the specified argument as seconds since
   1970-01-01 00:00 UTC and fractional part for its seconds and set
   those values into *SECONDS and *FRAC_VAL, storing the pointer to
   a following character into *ENDPTR. Return the number of set values,
   otherwise, -1 if a value is outside the range of its parameter.  */

int
sscanseconds (const char *argv,
              intmax_t *seconds, int *frac_val, char **endptr)
{
  struct numimax_prop seconds_prop = { 1, INTMAX_MIN, INTMAX_MAX, false };
  const char *p = argv;

  while (isspace (*p))
    p++;

  if (*p == '-')
    {
      seconds_prop.sign = -1;
      p++;
    }
  else if (*p == '+')
    p++;

  intmax_t sec;
  int set_num = sscannumimaxp (p, &seconds_prop, &sec, NULL, endptr);
  if (set_num > 0)
    {
      if (**endptr != '\0')
        {
          if (**endptr != '.' && **endptr != ',')
            return 0;

           /* If the fractional part is leading by the negative seconds,
              return its value added to 1.0 and must decrement seconds. */
           const struct numint_prop frac_prop =
             { seconds_prop.sign, 0, TM_FRAC_MAX, true };
           int sec_decr = 0;
           int frac_num = sscannumintp (*endptr + 1, &frac_prop,
                                        frac_val, &sec_decr, endptr);
           if (frac_num <= 0)
             return frac_num;
           else if (IMAX_SUBTRACT_WRAPV (sec, sec_decr, &sec))
             return -1;

           set_num++;
        }
      else
        *frac_val = 0;

      *seconds = sec;
    }

  return set_num;
}
