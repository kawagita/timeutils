/* sscantm.h -- Inputting parameters of time from standard input, converted
                from the numerical string

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

/* The property of time parsed from a string into the value of integer  */

struct tmint_prop
{
  /* If zero, ignore leading spaces and parse '-' or '+' as a sign,
     otherwise, parse only digits as its positive or negative value */
  int sign;
  int minval;
  int maxval;
  /* If greater than 0, parse a string as fractional part for the previous
     integer and skip digits over this value, and if the integer part is
     negative, decrement it and subtract the fractional part from 1.0 */
  int frac_digits;
  int delim;
};

/* Parse each part of the specified string as the value of integer into
   *TM_VALUES. Continue its parsing until the string is separated by '\0'
   or DELIM in struct tmint_prop, or Stop by the value which is overflow
   or outside the range of MINVAL to MAXVAL. Return the number of set
   values, or -1 if zero or one character except for digits is found.  */

int sscantmint (int *tm_values,
                const struct tmint_prop *tm_props, const char *str);

/* The property time parsed from a string into the value of intmax_t,
   including members same as struct tmint_prop  */

struct tmimax_prop
{
  int sign;
  intmax_t minval;
  intmax_t maxval;
  int frac_digits;
  int delim;
};

/* Parse each part of the specified string as the value of intmax_t into
   *TM_VALUES. Continue its parsing until the string is separated by '\0'
   or DELIM in struct tmint_prop, or Stop by the value which is overflow
   or outside the range of MINVAL to MAXVAL. Return the number of set
   values, or -1 if zero or one character except for digits is found.  */

int sscantmimax (intmax_t *tm_values,
                 const struct tmimax_prop *tm_props, const char *str);
