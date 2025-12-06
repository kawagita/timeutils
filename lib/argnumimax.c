/* Convert the leading part of the argument into an intmax_t number
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

#include "argnum.h"
#include "imaxoverflow.h"

#define ISDIGIT(c)  (c >= '0' && c <= '9')

/* Parse the leading part of the specified argument as an intmax_t number
   and set its value into *NUM_VAL, storing the pointer to a following
   character into *ENDPTR. Return 1 or 0 if a value is set or not,
   otherwise, -1 if outside the range of INT_MIN to INT_MAX.  */

int
argnumimax (const char *arg, intmax_t *num_val, char **endptr)
{
  struct numimax_prop imax_prop = { 0, INTMAX_MIN, INTMAX_MAX };

  return argnumimaxp (arg, &imax_prop, num_val, endptr);
}

/* Parse the leading part of the specified argument as an unsigned
   intmax_t number and set its value into *NUM_VAL, storing the pointer
   to a following character into *ENDPTR. Return 1 or 0 if a value is
   set or not, otherwise, -1 if outside the range of 0 to INT_MAX.  */

int
argnumuimax (const char *arg, intmax_t *num_val, char **endptr)
{
  struct numimax_prop uimax_prop = { 1, 0, INTMAX_MAX };

  return argnumimaxp (arg, &uimax_prop, num_val, endptr);
}

/* Parse the leading part of the specified argument as an intmax_t number
   and set its value into *NUM_VAL, storing the pointer to a following
   character into *ENDPTR. Return 1 or 0 if a value is set or not,
   otherwise, -1 if outside the range of the min_value to max_value member
   in *NUM_PROP.  */

int
argnumimaxp (const char *arg, struct numimax_prop *num_prop,
             intmax_t *num_val, char **endptr)
{
  const char *p = arg;
  int sign = num_prop->sign;

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

  intmax_t value = sign < 0 ? '0' - *p : *p - '0';
  p++;

  /* Convert leading digits of the argument into an intmax_t value. */
  while (ISDIGIT (*p))
    {
      if (IMAX_MULTIPLY_WRAPV (value, 10, &value)
          || IMAX_ADD_WRAPV (
               value, sign < 0 ? '0' - *p : *p - '0', &value))
        return -1;
      p++;
    }

  if (value < num_prop->min_value || value > num_prop->max_value)
    return -1;

  *endptr = (char *)p;
  *num_val = value;

  return 1;
}
