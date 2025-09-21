/* Convert a string to the value of integer for date and time
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
#include <stdint.h>

#include "intoverflow.h"
#include "sscantm.h"

#define ISDIGIT(c)  (c >= '0' && c <= '9')

/* Parse each part of the specified string as the value of integer into
   *TM_VALUES. Continue its parsing until the string is separated by '\0'
   or DELIM in struct tmint_prop, or Stop by the value which is overflow
   or outside the range of MINVAL to MAXVAL. Return the number of set
   values, or -1 if zero or one character except for digits is found.  */

int
sscantmint (int *tm_values,
            const struct tmint_prop *tm_props, const char *str)
{
  if (*str != '\0')
    {
      int set_num = 0;
      int sign;
      const char *p;

      do
        {
          const struct tmint_prop *tm_prop = &tm_props[set_num];
          int frac_digits = tm_prop->frac_digits;
          int value;
          bool overflow = false;

          p = str;

          /* Continue to use the sign of integer part if fractional. */
          if (set_num == 0 || frac_digits <= 0)
            sign = tm_prop->sign;

          /* Ignore leading spaces and parse '-' or '+' followed by a number
             as a sign if SIGN is 0, otherwise, parse only digits. */
          if (sign == 0)
            {
              while (isspace (*p))
                p++;

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
            return -1;

          value = sign < 0 && frac_digits <= 0 ? '0' - *p : *p - '0';
          p++;

          if (frac_digits > 0)
            {
              bool digit_parsed = true;
              int precision = 10;

              /* Accumulate the value of fractional part to the precision. */
              while (--frac_digits > 0)
                {
                  if ((sign < 0
                       && INT_MULTIPLY_WRAPV (precision, 10, &precision))
                      || INT_MULTIPLY_WRAPV (value, 10, &value))
                    break;
                  else if (digit_parsed)
                    {
                      if (ISDIGIT (*p))
                        {
                          if (INT_ADD_WRAPV (value, *p - '0', &value))
                            break;
                          p++;
                        }
                      else
                        digit_parsed = false;
                    }
                }

              if (sign < 0)
                {
                  /* Skip excess digits, truncating toward -Infinity. */
                  while (ISDIGIT (*p))
                    {
                      if (*p != '0')
                        {
                          /* Ignore the overflow by excess digits. */
                          INT_ADD_WRAPV (value, 1, &value);
                          break;
                        }
                      p++;
                    }

                  /* Decrement the value of integer part and change the sign
                     of fractional part by the subtraction from 1.0. */
                  if (set_num > 0
                      && (INT_SUBTRACT_WRAPV (precision, value, &value)
                          || INT_SUBTRACT_WRAPV (tm_values[set_num - 1],
                               1, &(tm_values[set_num - 1]))))
                    overflow = true;
                }
            }
          else  /* frac_digits <= 0 */
            {
              /* Convert digits in the head to the integer value. */
              while (ISDIGIT (*p))
                {
                  if (INT_MULTIPLY_WRAPV (value, 10, &value)
                      || INT_ADD_WRAPV (value,
                           sign < 0 ? '0' - *p : *p - '0', &value))
                    {
                      overflow = true;
                      break;
                    }
                  p++;
                }
            }

          /* Skip the rest of digits if overflow or over the precision. */
          while (ISDIGIT (*p))
            p++;

          /* If the string contains no or one character except for digits
             and a delimiter, return -1, otherwise if the value is overflow
             or outside the range, return the number to the previous part. */
          if (*p != tm_prop->delim && *p != '\0')
            return -1;
          else if (overflow || value < tm_prop->minval
                   || value > tm_prop->maxval)
            return set_num;

          tm_values[set_num++] = value;
          tm_prop++;
          str = p + 1;
        }
      while (*p != '\0');

      return set_num;
    }

  return -1;
}
