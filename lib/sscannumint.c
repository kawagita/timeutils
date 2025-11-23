/* Convert the leading part of the argument into an integer number
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
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "cmdtmio.h"
#include "intoverflow.h"

#define ISDIGIT(c)  (c >= '0' && c <= '9')

/* Parse the leading part of the specified argument as an integer number
   and set its value into *NUM_VAL, storing the pointer to a following
   character into *ENDPTR. Return 1 or 0 if a value is set or not,
   otherwise, -1 if outside the range of INT_MIN to INT_MAX.  */

int
sscannumint (const char *argv, int *num_val, char **endptr)
{
  const struct numint_prop int_prop = { 0, INT_MIN, INT_MAX, false };

  return sscannumintp (argv, &int_prop, num_val, NULL, endptr);
}

/* Parse the leading part of the specified argument as an unsigned
   integer number and set its value into *NUM_VAL, storing the pointer
   to a following character into *ENDPTR. Return 1 or 0 if a value is
   set or not, otherwise, -1 if outside the range of 0 to INT_MAX.  */

int
sscannumuint (const char *argv, int *num_val, char **endptr)
{
  const struct numint_prop uint_prop = { 0, 0, INT_MAX, false };

  return sscannumintp (argv, &uint_prop, num_val, NULL, endptr);
}

/* Parse the leading part of the specified argument as an integer number
   and set its value into *NUM_VAL, storing the pointer to a following
   character into *ENDPTR. If INTDECR is not NULL, add 1.0 to the value
   and set 1 into *INTDECR when the negative fractional part is parsed,
   otherwise, set 0. Return 1 or 0 if a value is set or not, otherwise,
   -1 if outside the range of the min_value to max_value member in
   *NUM_PROP.  */

int
sscannumintp (const char *argv, const struct numint_prop *num_prop,
              int *num_val, int *intdecr, char **endptr)
{
  while (isspace (*argv))
    argv++;

  *endptr = (char *)argv;

  const char *p = argv;
  int sign = num_prop->sign;

  /* Parse '-' or '+' followed by a number as a sign if SIGN is 0,
     otherwise, parse only digits. */
  if (sign == 0)
    {
      if (*p == '-')
        {
          sign = -1;
          p++;
        }
      else if (*p == '+')
        {
          sign = 1;
          p++;
        }
    }

  if (! ISDIGIT (*p))
    return 0;

  int value = sign < 0 && !num_prop->isfrac ? '0' - *p : *p - '0';
  p++;

  if (num_prop->isfrac)
    {
      bool digit_parsed = true;
      int frac_digits = TM_FRAC_DIGITS;

      /* Accumulate the value of fractional part to the precision. */
      while (--frac_digits > 0)
        {
          if (INT_MULTIPLY_WRAPV (value, 10, &value))
            return -1;
          else if (digit_parsed)
            {
              if (ISDIGIT (*p))
                {
                  if (INT_ADD_WRAPV (value, *p - '0', &value))
                    return -1;
                  p++;
                }
              else
                digit_parsed = false;
            }
        }

      if (intdecr)
        {
          if (sign < 0)
            {
              /* Skip excess digits, truncating toward -Infinity. */
              while (ISDIGIT (*p))
                {
                  if (*p != '0')
                    {
                      /* Ignore the overflow by excess digits. */
                      if (value < INT_MAX)
                        value++;

                      break;
                    }
                  p++;
                }

              /* Add 1.0 and change fractional value to the positive,
                 and set the decrement of integer part. */
              value = TM_FRAC_MAX - value + 1;
              *intdecr = 1;
            }
          else
            *intdecr = 0;
        }
      else if (sign < 0)
        value = - value;

      /* Skip the rest of digits if over the precision. */
      while (ISDIGIT (*p))
        p++;
    }
  else  /* !num_prop->isfrac */
    {
      /* Convert leading digits of the argument into an integer value. */
      while (ISDIGIT (*p))
        {
          if (INT_MULTIPLY_WRAPV (value, 10, &value)
              || INT_ADD_WRAPV (
                   value, sign < 0 ? '0' - *p : *p - '0', &value))
            return -1;
          p++;
        }
    }

  if (value < num_prop->min_value || value > num_prop->max_value)
    return -1;

  while (isspace (*p))
    p++;

  *endptr = (char *)p;
  *num_val = value;

  return 1;
}
