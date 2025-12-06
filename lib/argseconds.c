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

#include "argempty.h"
#include "argnum.h"
#include "ftsec.h"
#include "imaxoverflow.h"

extern const char *arg_endptr;

/* Parse the leading part of the specified argument as seconds since
   1970-01-01 00:00 UTC and fractional part for its seconds and set
   those values into *SECONDS and *NSEC, storing the pointer to
   a following character into *ENDPTR. Return the number of set values,
   otherwise, -1 if a value is outside the range of its parameter.  */

int
argseconds (const char *arg, intmax_t *seconds, int *nsec, char **endptr)
{
  struct numimax_prop sec_prop = { 1, INTMAX_MIN, INTMAX_MAX };
  const char *p = arg;
  char *endp;

  *endptr = (char *)p;

  if (*p == '-')
    {
      sec_prop.sign = -1;
      p++;
    }
  else if (*p == '+')
    p++;

  intmax_t sec;
  int set_num = argnumimaxp (p, &sec_prop, &sec, &endp);
  if (set_num > 0)
    {
      if (! argempty (endp))
        {
          if (*endp != '.' && *endp != ',')
            return 0;

           struct numint_prop ns_prop =
             { sec_prop.sign, 0, FT_NSEC_PRECISION - 1, FT_NSEC_DIGITS, &sec };
           int ns;
           int ns_num = argnumintp (endp + 1, &ns_prop, &ns, &endp);
           if (ns_num < 0)
             return -1;
           else if (ns_num == 0 || ! argempty (endp))
             return 0;

           *nsec = ns;
           set_num++;
        }

      *seconds = sec;
      *endptr = (char *)arg_endptr;
    }

  return set_num;
}
