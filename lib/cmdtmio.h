/* cmdtmio.h -- Inputting or outputting parameters of time from or to
                the command line

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

/* Pointers to parameters of time input from arguments or output to
   standard output in the command line  */

struct tm_ptrs
{
  int **dates;
  int *weekday;
  intmax_t *weekday_ordinal;
  int *yearday;
  int **times;
  intmax_t **rel_times;
  int *ns;
  long int *utcoff;
};

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

int argtmiso8601 (const char *arg, struct tm_ptrs *tm_ptrs, char **endptr);

/* Parse the specified arguments as the relative year, month, day, hour,
   minutes, seconds, and nanoseconds in order and set those values into
   members included in *TM_PTRS, storing the pointer to an error argument
   into *ERRARG if not set. Return the number of set values, otherwise, -1
   if a value is outside the range of its parameter.  */

int argreltm (int argc, const char **argv,
              struct tm_ptrs *tm_ptrs, char **errarg);

/* Parse the leading part of the specified argument as seconds since
   1970-01-01 00:00 UTC and fractional part for its seconds and set
   those values into *SECONDS and *NSEC, storing the pointer to
   a following character into *ENDPTR. Return the number of set values,
   otherwise, -1 if a value is outside the range of its parameter.  */

int argseconds (const char *arg, intmax_t *seconds, int *nsec, char **endptr);

/* Parse the leading part of the specified argument as the week day name
   and ordinal separated by a comma and set those values into *WEEKDAY
   and *WEEKDAY_ORDINAL, storing the pointer to a following character into
   *ENDPTR. Return the number of set values, otherwise, -1 if the week day
   ordinal is outside the range of 0 to INTMAX_MAX.  */

int argweekday (const char *arg,
                int *weekday, intmax_t *weekday_ordinal, char **endptr);

/* The name of settings whether the time is adjusted by DST offset in
   a time zone  */

#define DST_NAME "DST"
#define ST_NAME  "ST"

#define DST_ST_NOTATION "\"DST\"|\"ST\""

/* Parse the leading part of the specified argument as the setting name
   whether the time is adjusted by DST offset in a time zone and set its
   value into *ISDST. Return true if "DST" or "ST".  */

bool argisdst (const char *arg, int *isdst);

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

/* Output parameters of date or time included in *TM_PTRS to standard
   output, according to the format defined for each flag set in *TM_FMT.
   Return the number of output parameters.  */

int printtm (const struct tm_fmt *tm_fmt, const struct tm_ptrs *tm_ptrs);

/* Output parameters of date and time included in *TM_PTRS with a leading
   space in order to standard output. If NO_NEWLINE is true, output
   the trailing newline. Return the number of output parameters.  */

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
