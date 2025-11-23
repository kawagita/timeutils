/* cmdtmio.h -- Inputting or outputting a number or parameters of time
                from or to the command line

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
  /* If true, parse the string as fractional part for a leading integer
     and skip digits over this value */
  bool isfrac;
};

/* Parse the leading part of the specified argument as an integer number
   and set its value into *NUM_VAL, storing the pointer to a following
   character into *ENDPTR. Return 1 or 0 if a value is set or not,
   otherwise, -1 if outside the range of INT_MIN to INT_MAX.  */

int sscannumint (const char *argv, int *num_val, char **endptr);

/* Parse the leading part of the specified argument as an unsigned
   integer number and set its value into *NUM_VAL, storing the pointer
   to a following character into *ENDPTR. Return 1 or 0 if a value is
   set or not, otherwise, -1 if outside the range of 0 to INT_MAX.  */

int sscannumuint (const char *argv, int *num_val, char **endptr);

/* Parse the leading part of the specified argument as an integer number
   and set its value into *NUM_VAL, storing the pointer to a following
   character into *ENDPTR. If INTDECR is not NULL, add 1.0 to the value
   and set 1 into *INTDECR when the negative fractional part is parsed,
   otherwise, set 0. Return 1 or 0 if a value is set or not, otherwise,
   -1 if outside the range of the min_value to max_value member in
   *NUM_PROP.  */

int sscannumintp (const char *argv, const struct numint_prop *num_prop,
                  int *num_val, int *intdecr, char **endptr);

/* The property of an intmax_t number parsed from the string,
   including members same as struct numint_prop  */

struct numimax_prop
{
  int sign;
  intmax_t min_value;
  intmax_t max_value;
  bool isfrac;
};

/* Parse the leading part of the specified argument as an intmax_t number
   and set its value into *NUM_VAL, storing the pointer to a following
   character into *ENDPTR. Return 1 or 0 if a value is set or not,
   otherwise, -1 if outside the range of INT_MIN to INT_MAX.  */

int sscannumimax (const char *argv, intmax_t *num_val, char **endptr);

/* Parse the leading part of the specified argument as an unsigned
   intmax_t number and set its value into *NUM_VAL, storing the pointer
   to a following character into *ENDPTR. Return 1 or 0 if a value is
   set or not, otherwise, -1 if outside the range of 0 to INT_MAX.  */

int sscannumuimax (const char *argv, intmax_t *num_val, char **endptr);

/* Parse the leading part of the specified argument as an intmax_t number
   and set its value into *NUM_VAL, storing the pointer to a following
   character into *ENDPTR. If INTDECR is not NULL, add 1.0 to the value
   and set 1 into *INTDECR when the negative fractional part is parsed,
   otherwise, set 0. Return 1 or 0 if a value is set or not, otherwise,
   -1 if outside the range of the min_value to max_value member in
   *NUM_PROP.  */

int sscannumimaxp (const char *argv, const struct numimax_prop *num_prop,
                   intmax_t *num_val, int *intdecr, char **endptr);

/* The table of a word name and value  */

struct word_table
{
  const char *name;  /* Must be NULL if not followed by the next word */
  int value;
};

/* Compare the leading part of the specified argument with a name member
   in *TABLE case-insensitively and set its value member into *VALUE
   if equal to the whole or ABBRLEN characters, storing the pointer to
   a following character into the *ENDPTR. Continue to compare with
   following tables unless the name member is not NULL. Return 1 or 0
   if a value is set or not.  */

int sscanword (const char *argv, const struct word_table *table,
               int abbrlen, int *value, char **endptr);

/* Pointers to parameters of time input from standard input or output to
   standard output in the command line  */

struct tm_ptrs
{
  int **dates;
  int *weekday;
  intmax_t *weekday_ordinal;
  int *yearday;
  int **times;
  intmax_t **rel_times;
  int *frac_val;
  long int *utcoff;
};

#ifndef TM_YEAR_BASE
# define TM_YEAR_BASE 1900
#endif

/* The maximum value and digits of fractional part in seconds  */

#define TM_FRAC_MAX    9999999
#define TM_FRAC_DIGITS       7

/* Parse the leading part of the specified argument as parameters of time
   included in *TM_PTRS and set those values into the member, storing
   the pointer to a following character into *ENDPTR. Return the number
   of set values, otherwise, -1 if a value is outside the range of its
   parameter.  */

int sscantm (const char *argv, struct tm_ptrs *tm_ptrs, char **endptr);

/* Parse the specified arguments as relative parameters of time included
   in *TM_PTRS and set those values into the member, storing the pointer
   to a following character into *ENDPTR. Return the number of set values,
   otherwise, -1 if a value is outside the range of its parameter.  */

int sscanreltm (int argc, const char **argv,
                struct tm_ptrs *tm_ptrs, char **endptr);

/* Parse the leading part of the specified argument as seconds since
   1970-01-01 00:00 UTC and fractional part for its seconds and set
   those values into *SECONDS and *FRAC_VAL, storing the pointer to
   a following character into *ENDPTR. Return the number of set values,
   otherwise, -1 if a value is outside the range of its parameter.  */

int sscanseconds (const char *argv,
                  intmax_t *seconds, int *frac_val, char **endptr);

/* Parse the leading part of the specified argument as the week day name
   and ordinal separated by a comma and set those values into *WEEKDAY
   and *WEEKDAY_ORDINAL, storing the pointer to a following character into
   *ENDPTR. Return 1 or 0 if a value is set or not.  */

int sscanweekday (const char *argv,
                  int *weekday, intmax_t *weekday_ordinal, char **endptr);

/* The name of settings whether the time is adjusted by DST offset  */

#define DST_NAME "DST"
#define ST_NAME  "ST"

/* Parse the leading part of the specified argument as the setting name
   whether the time is adjusted by DST offset and set its value into
   *ISDST, storing the pointer to a following character into *ENDPTR.
   Return 1 or 0 if a value is set or not.  */

int sscanisdst (const char *argv, int *isdst, char **endptr);

/* The description in Unix seconds or file time  */

#define IN_UNIX_SECONDS "in seconds since 1970-01-01 00:00 UTC"
#define IN_FILETIME     "in 100 nanoseconds since 1601-01-01 00:00 UTC"

#ifdef USE_TM_GLIBC
# define IN_DEFAULT_TIME IN_UNIX_SECONDS
#else
# define IN_DEFAULT_TIME IN_FILETIME
#endif

/* Output the usage of the specified program name and description to
   standard output. If HAS_OPTIONS is true, output "[OPTION]..." in
   the back of *NAME, and if HAS_ISDST is true or TRANS_NO_DST_OPTION
   is an alphabet option, output the description of adjusting by DST
   offset in the ordinary or transition date.  */

void printusage (const char *name, const char *desc,
                 bool has_options, bool has_isdst, int trans_no_dst_option);

/* The format of time output to standard output  */

struct tm_fmt
{
  bool weekday_name;
  bool week_numbering;
  bool iso8601;
  bool japanese;
  bool no_newline;
};

/* The format of fractional part in seconds  */

#define TM_FRAC_FORMAT ".%07d"

/* Output parameters of time included in *TM_PTRS to standard output,
   according to the format defined for each flag set in *TM_FMT. Return
   the number of output parameters.  */

int printtm (const struct tm_fmt *tm_fmt, const struct tm_ptrs *tm_ptrs);

/* Output relative parameters of time included in *TM_PTRS with a leading
   space to standard output. If NO_NEWLINE is true, output the trailing
   newline. Return the number of output parameters.  */

int printreltm (bool no_newline, const struct tm_ptrs *tm_ptrs);

/* Output the specified seconds or nanoseconds elapsed since a time to
   standard output. If FLAC_VAL is not less than 0 or NO_NEWLINE is true,
   output its fractional value or the trailing newline. Return 1.  */

int printelapse (bool no_newline, intmax_t elapse, int frac_val);

/* Output the setting name whether the time is adjusted by DST offset
   in time zone for the specified flag with a leading space to standard
   output. If NO_NEWLINE is true, output the trailing newline. Return
   1 if the flag is not less than 0, otherwise, 0.  */

int printisdst (bool no_newline, int isdst);
