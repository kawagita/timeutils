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

#include "argnum.h"
#include "imaxoverflow.h"
#include "intoverflow.h"

#define ISDIGIT(c)  (c >= '0' && c <= '9')

/* Parse the leading part of the specified argument as an integer number
   and set its value into *NUM_VAL, storing the pointer to a following
   character into *ENDPTR. Return 1 or 0 if a value is set or not,
   otherwise, -1 if outside the range of INT_MIN to INT_MAX.  */

int
argnumint (const char *arg, int *num_val, char **endptr)
{
  struct numint_prop int_prop = { 0, INT_MIN, INT_MAX, 0, NULL };

  return argnumintp (arg, &int_prop, num_val, endptr);
}

/* Parse the leading part of the specified argument as an unsigned
   integer number and set its value into *NUM_VAL, storing the pointer
   to a following character into *ENDPTR. Return 1 or 0 if a value is
   set or not, otherwise, -1 if outside the range of 0 to INT_MAX.  */

int
argnumuint (const char *arg, int *num_val, char **endptr)
{
  struct numint_prop uint_prop = { 1, 0, INT_MAX, 0, NULL };

  return argnumintp (arg, &uint_prop, num_val, endptr);
}

/* Parse the leading part of the specified argument as an integer number
   and set its value into *NUM_VAL, storing the pointer to a following
   character into *ENDPTR. Return 1 or 0 if a value is set or not,
   otherwise, -1 if outside the range of the min_value to max_value member
   in *NUM_PROP.  */

int
argnumintp (const char *arg, struct numint_prop *num_prop,
            int *num_val, char **endptr)
{
  const char *p = arg;
  int sign = num_prop->sign;
  intmax_t *decr_int_value = NULL;

  *endptr = (char *)p;

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

  int frac_digits = num_prop->frac_digits;
  int value = sign < 0 && frac_digits <= 0 ? '0' - *p : *p - '0';
  p++;

  if (frac_digits > 0)
    {
      bool digit_parsed = true;
      int precision = 10;

      /* Accumulate the value of fractional part to the precision. */
      while (--frac_digits > 0)
        {
          if ((sign < 0 && INT_MULTIPLY_WRAPV (precision, 10, &precision))
              || INT_MULTIPLY_WRAPV (value, 10, &value))
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

      if (num_prop->int_value)
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

              /* Decrement the value of integer part in *INT_VALUE
                 and add its 1 to fractional value. */
              decr_int_value = num_prop->int_value;
              value = precision - value;
            }
        }
      else if (sign < 0)
        value = - value;

      /* Skip the rest of digits if over the precision. */
      while (ISDIGIT (*p))
        p++;
    }
  else  /* frac_digits <= 0 */
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

  if (value < num_prop->min_value || value > num_prop->max_value
      || (decr_int_value
          && IMAX_SUBTRACT_WRAPV (*decr_int_value, 1, decr_int_value)))
    return -1;

  *endptr = (char *)p;
  *num_val = value;

  return 1;
}
