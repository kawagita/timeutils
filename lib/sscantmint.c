/* Convert a string into values of integer for date and time
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
#include <stdint.h>

#include "intoverflow.h"
#include "sscantm.h"

#define ISDIGIT(c)  (c >= '0' && c <= '9')

/* Parse the specified string as the integer number and set its value
   into *TM_VALUES, storing the pointer to '\0' or a failed character
   into the *ENDPTR. Return the nubmer of set values, or -1 if a parsed
   value is overflow.  */

int
sscantmint (const char *str, int *tm_value, char **endptr)
{
  const struct tmint_prop tmint_spec_prop = { 0, INT_MIN, INT_MAX, 0, '\0' };

  return sscantmintp (str, &tmint_spec_prop, &tm_value, endptr);
}

/* Parse a leading part in the specified string as the integer number
   separated by DELIM in struct tmint_prop and set its value into
   **TM_VALP, storing the pointer to '\0' or a failed character into
   the *ENDPTR. Continue to parse the string for numbers specified by
   *TM_PROPS until parsing is terminated by '\0'. Return the nubmer of
   set values, or -1 if a parsed value is outside the range of MIN_VALUE
   to MAX_VALUE.  */

int
sscantmintp (const char *str, const struct tmint_prop *tm_props,
            int **tm_valp, char **endptr)
{
  int set_num = 0;
  int sign;
  const char *p;
  const struct tmint_prop *tm_prop;

  do
    {
      tm_prop = &tm_props[set_num];

      int frac_digits = tm_prop->frac_digits;
      int value;

      p = str;

      while (isspace (*p))
        p++;

      /* Use the sign same as the previous integer part if fractional. */
      if (set_num == 0 || frac_digits <= 0)
        sign = tm_prop->sign;

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
        {
          if (set_num > 0)
            str--;  /* back to the previous delimiter */
          p = str;
          break;
        }

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
                      || INT_SUBTRACT_WRAPV (
                           *tm_valp[set_num - 1], 1, tm_valp[set_num - 1])))
                return -1;
            }

            /* Skip the rest of digits if over the precision. */
            while (ISDIGIT (*p))
              p++;
        }
      else  /* frac_digits <= 0 */
        {
          /* Convert leading digits in the string into an integer value. */
          while (ISDIGIT (*p))
            {
              if (INT_MULTIPLY_WRAPV (value, 10, &value)
                  || INT_ADD_WRAPV (
                       value, sign < 0 ? '0' - *p : *p - '0', &value))
                return -1;
              p++;
            }
        }

      if (value < tm_prop->min_value || value > tm_prop->max_value)
        return -1;

      *tm_valp[set_num++] = value;

      while (isspace (*p))
        p++;

      /* Continue to parse the following string if DELIM is not '\0'
         and appeared, otherwise, set *ENDPTR to '\0' or a failed
         character and return the number including the current part. */
      str = p + 1;
    }
  while (*p != '\0' && *p == tm_prop->delim);

  *endptr = (char *)p;

  return set_num;
}
