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

/* The property of time parsed from a string to the number of integer  */

struct tmint_prop
{
  /* If zero, parse '-' or '+' as a sign, otherwise, parse only digits
     as the positive or negative value */
  int sign;
  int min_value;
  int max_value;
  /* If greater than 0, parse a string as fractional part for the previous
     integer and skip digits over this value, and if the integer part is
     negative, decrement it and subtract the fractional part from 1.0 */
  int frac_digits;
  int delim;  /* Must be '\0' if not followed by the next number */
};

/* Parse the specified string as the integer number and set its value
   into *TM_VALUES, storing the pointer to '\0' or a failed character
   into the *ENDPTR. Return the nubmer of set values, or -1 if a parsed
   value is overflow.  */

int sscantmint (const char *str, int *tm_value, char **endptr);

/* Parse a leading part in the specified string as the integer number
   separated by DELIM in struct tmint_prop and set its value into
   **TM_VALP, storing the pointer to '\0' or a failed character into
   the *ENDPTR. Continue to parse the string for numbers specified by
   *TM_PROPS until parsing is terminated by '\0'. Return the nubmer of
   set values, or -1 if a parsed value is outside the range of MIN_VALUE
   to MAX_VALUE.  */

int sscantmintp (const char *str, const struct tmint_prop *tm_props,
                 int **tm_valp, char **endptr);

/* The property time parsed from a string to the number of intmax_t,
   including members same as struct tmint_prop  */

struct tmimax_prop
{
  int sign;
  intmax_t min_value;
  intmax_t max_value;
  int frac_digits;
  int delim;
};

/* Parse the specified string as the intmax_t number and set its value
   into *TM_VALUES, storing the pointer to '\0' or a failed character
   into the *ENDPTR. Return the nubmer of set values, or -1 if a parsed
   value is overflow.  */

int sscantmimax (const char *str, intmax_t *tm_value, char **endptr);

/* Parse a leading part in the specified string as the intmax_t number
   separated by DELIM in struct tmint_prop and set its value into
   *TM_VALP, storing the pointer to '\0' or a failed character into
   the *ENDPTR. Continue to parse the string for numbers specified by
   *TM_PROPS until parsing is terminated by '\0'. Return the nubmer of
   set values, or -1 if a parsed value is outside the range of MIN_VALUE
   to MAX_VALUE.  */

int sscantmimaxp (const char *str, const struct tmimax_prop *tm_props,
                  intmax_t **tm_valp, char **endptr);
