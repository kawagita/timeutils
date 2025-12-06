/* argnum.h -- Getting a value for number string from the argument

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

/* The property of an integer number parsed from the string  */

struct numint_prop
{
  /* If zero, parse '-' or '+' as a sign, otherwise, parse only digits
     as the positive or negative value */
  int sign;
  int min_value;
  int max_value;
  /* If greater than 0, parse the string as fractional part for a leading
     integer and skip digits over this. And if its value is negative, add
     1 subtracted from *INT_VALUE unless not NULL */
  int frac_digits;
  intmax_t *int_value;
};

/* Parse the leading part of the specified argument as an integer number
   and set its value into *NUM_VAL, storing the pointer to a following
   character into *ENDPTR. Return 1 or 0 if a value is set or not,
   otherwise, -1 if outside the range of INT_MIN to INT_MAX.  */

int argnumint (const char *arg, int *num_val, char **endptr);

/* Parse the leading part of the specified argument as an unsigned
   integer number and set its value into *NUM_VAL, storing the pointer
   to a following character into *ENDPTR. Return 1 or 0 if a value is
   set or not, otherwise, -1 if outside the range of 0 to INT_MAX.  */

int argnumuint (const char *arg, int *num_val, char **endptr);

/* Parse the leading part of the specified argument as an integer number
   and set its value into *NUM_VAL, storing the pointer to a following
   character into *ENDPTR. Return 1 or 0 if a value is set or not,
   otherwise, -1 if outside the range of the min_value to max_value member
   in *NUM_PROP.  */

int argnumintp (const char *arg, struct numint_prop *num_prop,
                int *num_val, char **endptr);

/* The property of an intmax_t number parsed from the string  */

struct numimax_prop
{
  int sign;
  intmax_t min_value;
  intmax_t max_value;
};

/* Parse the leading part of the specified argument as an intmax_t number
   and set its value into *NUM_VAL, storing the pointer to a following
   character into *ENDPTR. Return 1 or 0 if a value is set or not,
   otherwise, -1 if outside the range of INT_MIN to INT_MAX.  */

int argnumimax (const char *arg, intmax_t *num_val, char **endptr);

/* Parse the leading part of the specified argument as an unsigned
   intmax_t number and set its value into *NUM_VAL, storing the pointer
   to a following character into *ENDPTR. Return 1 or 0 if a value is
   set or not, otherwise, -1 if outside the range of 0 to INT_MAX.  */

int argnumuimax (const char *arg, intmax_t *num_val, char **endptr);

/* Parse the leading part of the specified argument as an intmax_t number
   and set its value into *NUM_VAL, storing the pointer to a following
   character into *ENDPTR. Return 1 or 0 if a value is set or not,
   otherwise, -1 if outside the range of the min_value to max_value member
   in *NUM_PROP.  */

int argnumimaxp (const char *arg, struct numimax_prop *num_prop,
                 intmax_t *num_val, char **endptr);
