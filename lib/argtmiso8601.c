/* Input parameters of time from an argument in ISO 8601 format
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
#include "argnum.h"
#include "cmdtmio.h"
#include "ftsec.h"

extern const char *arg_endptr;

/* Parse the leading part of the specified argument as ISO 8601 format
   and set those values into members included in *TM_PTRS, storing
   the pointer to a following character into *ENDPTR. The date, time,
   and/or UTC offset is input in ISO 8601 format by below notation;

   [YYYY-MM-DD][Thh:mm:dd[.nnnnnnn]][Z|+hhmm|-hhmm]

   Each part must conform to this format when specified, and if so,
   return the number of set values. Otherwise, if incorrect format,
   return 0, or if a value is outside the range of its parameter,
   return -1. However specially accept "Z+hhmm" and "Z-hhmm" in UTC
   offset if only it is given.  */

int
argtmiso8601 (const char *arg, struct tm_ptrs *tm_ptrs, char **endptr)
{
  const char *p = arg;
  char *endp;
  int i;
  int set_num = 0;
  int date_num = 0;
  int time_num = 0;
  int dates[3];
  int times[3];
  int ns = -1;

  /* Input the year, month, and day. */
  if (tm_ptrs->dates && *p != 'T' && *p != 'Z')
    {
      for (i = 0; i < 3; i++)
        {
          int sign = 1;

          *endptr = (char *)p;

          if (*p == '-' || *p == '+')
            {
              /* Set the month or day of +MM or +DD to a negative value. */
              if (i > 0)
                {
                  if (*p == '+')
                    sign = -1;
                }
              else if (*p == '-')
                sign = -1;
              p++;
            }

          date_num = argnumuint (p, dates + i, &endp);
          if (date_num <= 0)
            return date_num;
          else if (sign < 0)
            dates[i] = - dates[i];

          set_num++;
          p = endp;
        }
    }

  /* Intput the hour, minutes, and seconds. */
  if (tm_ptrs->times && *p == 'T')
    {
      for (i = 0; i < 3; i++)
        {
          *endptr = (char *)p;

          if (i > 0 && *p != ':')
            return 0;

          time_num = argnumuint (p + 1, times + i, &endp);
          if (time_num <= 0)
            return time_num;

          set_num++;
          p = endp;
        }

      /* Input the nanoseconds less than a second. */
      if (tm_ptrs->ns && (*p == '.' || *p == ','))
        {
          *endptr = (char *)p;

          struct numint_prop ns_prop =
            { 1, 0, FT_NSEC_PRECISION, FT_NSEC_DIGITS, NULL };
          int ns_num = argnumintp (p + 1, &ns_prop, &ns, &endp);
          if (ns_num <= 0)
            return ns_num;

          set_num++;
          p = endp;
        }
    }

  /* Intput the UTC offset in a time zone. */
  if (tm_ptrs->utcoff && *p != '\0')
    {
      bool leading_z = false;

      *endptr = (char *)p;

      if (*p == 'Z')
        {
          leading_z = true;
          p++;
        }

      if (*p != '-' && *p != '+')
        {
          if (! argempty (p))
            return 0;
          else if (leading_z)  /* YYYY-MM-DDZ, Thh:mm:ssZ, or Z */
            {
              *tm_ptrs->utcoff = 0;
              set_num++;
            }
        }
      else if (! leading_z || set_num == 0)
        {
          /* Set the UTC offset of Z+hhmm or Z-hhmm in the argument
             to the value calculated from +hhmm or -hhmm, ignoring 'Z'
             which is only used to distinguish it from date format. */
          int hhmm_val = 0;
          int utcoff_num = argnumint (p, &hhmm_val, &endp);
          if (utcoff_num < 0 || hhmm_val < -2400 || hhmm_val > 2400)
            return -1;
          else if (utcoff_num == 0 || ! argempty (endp))
            return 0;

          *tm_ptrs->utcoff = ((hhmm_val / 100) * 60 + hhmm_val % 100) * 60;
          set_num++;
        }
      else  /* YYYY-MM-DDZ+hhmm or Thh:mm:ssZ-hhmm */
        return 0;

      p = arg_endptr;
    }
  else if (! argempty (p))
    return 0;

  if (date_num > 0)
    {
      for (i = 0; i < 3; i++)
        *tm_ptrs->dates[i] = dates[i];
    }
  if (time_num > 0)
    {
      for (i = 0; i < 3; i++)
        *tm_ptrs->times[i] = times[i];
    }
  if (ns >= 0)
    *tm_ptrs->ns = ns;

  *endptr = (char *)p;

  return set_num;
}
