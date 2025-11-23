/* Convert parameters of time into Unix seconds
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
   awith this program; if not, write to the Free Software Foundation,
   Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.  */

#include "config.h"

#ifndef USE_TM_GLIBC
# include <windows.h>
#endif
#include <stdbool.h>
#include <stdint.h>

#ifndef USE_TM_SELFIMPL
# include <time.h>

# ifdef USE_TM_MSVCRT
long int tm_diff (struct tm const *a, struct tm const *b);
# endif
#endif

#include "wintm.h"

#ifdef USE_TM_SELFIMPL
# include "adjusttm.h"
# include "adjusttz.h"
# include "imaxoverflow.h"
# include "intoverflow.h"
# include "timeoverflow.h"
#endif

/* If DST is in effect or not for a time that is either skipped over or
   repeated when a transition to or from DST occurs, specify a positive
   value or zero, otherwise, attempt to determine whether the specified
   time is included in the term of DST  */
#ifdef TEST
static int trans_isdst = 1;
#else
extern int trans_isdst;
#endif

/* Convert the specified parameters of *TM into seconds since Unix epoch,
   and adjust each parameter to the range of correct values. If successful,
   return the number of converted seconds and overwrite *TM by adjusted
   values, otherwise, return -1 and never overwrite those.  */

intmax_t
mktimew (TM *tm)
{
#ifndef USE_TM_SELFIMPL
# ifdef USE_TM_GLIBC
  return mktime (tm);
# else  /* USE_TM_MSVCRT */
  time_t t;
  struct tm gmt, lct;

  lct.tm_year = tm->tm_year;
  lct.tm_mon = tm->tm_mon;
  lct.tm_mday = tm->tm_mday;
  lct.tm_hour = tm->tm_hour;
  lct.tm_min = tm->tm_min;
  lct.tm_sec = tm->tm_sec;
  lct.tm_wday = tm->tm_wday;
  lct.tm_yday = tm->tm_yday;
  lct.tm_isdst = tm->tm_isdst;

  t = mktime (&lct);

  if (lct.tm_wday >= 0)
    {
      tm->tm_year = lct.tm_year;
      tm->tm_mon = lct.tm_mon;
      tm->tm_mday = lct.tm_mday;
      tm->tm_hour = lct.tm_hour;
      tm->tm_min = lct.tm_min;
      tm->tm_sec = lct.tm_sec;
      tm->tm_wday = lct.tm_wday;
      tm->tm_yday = lct.tm_yday;
      tm->tm_isdst = lct.tm_isdst;
      if (! gmtime_s (&gmt, &t) && gmt.tm_wday >= 0)
        tm->tm_gmtoff = tm_diff (&lct, &gmt);
    }

  return t;
# endif
#else  /* USE_TM_SELFIMPL */
  struct dtm date;
  intmax_t seconds;
  intmax_t epochday;
  int epochyears;
  int year;
  int hour = tm->tm_hour;
  int min = tm->tm_min;
  int sec = tm->tm_sec;

  date.tm_year = tm->tm_year;
  date.tm_mon = tm->tm_mon;
  date.tm_mday = tm->tm_mday;
  date.tm_wday = 0; /* No calculation of the week day */

  /* Adjust parameters of date and time to the range of correct values. */
  if (! carrytm (&min, &sec, 60) || ! carrytm (&hour, &min, 60)
      || ! carrytm (&date.tm_mday, &hour, 24) || ! adjustday (&date))
    return -1;

  if (INT_ADD_WRAPV (date.tm_year, TM_YEAR_BASE, &year)
      || INT_SUBTRACT_WRAPV (year, UNIXEPOCH_YEAR, &epochyears))
    return -1;

  /* Calcuate the day number since Unix epoch including some leap days
     and set the week day to the remainder of dividing its value by 7. */
  int ldays = 0;

  if (year ^ UNIXEPOCH_YEAR)
    {
      int from_year = UNIXEPOCH_YEAR;
      int to_year = year;
      if (year > UNIXEPOCH_YEAR)
        to_year--;
      else
        from_year--;

      ldays = leapdays (from_year, to_year);
    }

  if (IMAX_MULTIPLY_WRAPV (epochyears, DAYS_IN_YEAR, &epochday)
      || (ldays && IMAX_ADD_WRAPV (epochday, ldays, &epochday))
      || IMAX_ADD_WRAPV (epochday, date.tm_yday, &epochday))
    return -1;

  date.tm_wday = WEEKDAY_FROM (UNIXEPOCH_WEEKDAY, epochday);

  /* Add the number of seconds converted from time in a day to Unix seconds. */
  if (IMAX_MULTIPLY_WRAPV (epochday, SECONDS_IN_DAY, &seconds)
      || IMAX_ADD_WRAPV (seconds, SECONDS_AT (hour, min, sec), &seconds)
      || timew_overflow (seconds))
    return -1;

  /* Adjust parameters of time for the increase or decrease of minutes by
     the offset of time zone to the range of correct values. */
  struct lctm lct;
  intmax_t lct_offset = 0;

  lct.tm_year = date.tm_year;
  lct.tm_ysec = date.tm_yday * SECONDS_IN_DAY + SECONDS_AT (hour, min, sec);
  lct.tm_min = min;
  lct.tm_isdst = tm->tm_isdst;

  if (! adjusttz (&lct, trans_isdst))
    return -1;

  int adj_min = lct.tm_min - min;
  if (adj_min)
    {
      int adj_day = 0;

      min = lct.tm_min;
      if (IMAX_MULTIPLY_WRAPV (adj_min, 60, &lct_offset)
          || ! carrytm (&hour, &min, 60) || ! carrytm (&adj_day, &hour, 24))
        return -1;

      if (adj_day)
        {
          if (INT_ADD_WRAPV (date.tm_mday, adj_day, &date.tm_mday)
              || ! adjustday (&date))
            return -1;

          date.tm_wday = WEEKDAY_FROM (date.tm_wday, adj_day);
        }
    }

  /* Subtract the offset of time zone from UTC seconds since Unix epoch. */
  if (IMAX_SUBTRACT_WRAPV (lct_offset, lct.tm_gmtoff, &lct_offset)
      || (lct_offset && (IMAX_ADD_WRAPV (seconds, lct_offset, &seconds)
                     || timew_overflow (seconds))))
    return -1;

  tm->tm_year = date.tm_year;
  tm->tm_mon = date.tm_mon;
  tm->tm_mday = date.tm_mday;
  tm->tm_hour = hour;
  tm->tm_min = min;
  tm->tm_sec = sec;
  tm->tm_wday = date.tm_wday;
  tm->tm_yday = date.tm_yday;
  tm->tm_isdst = lct.tm_isdst;
  tm->tm_gmtoff = lct.tm_gmtoff;

  return seconds;
#endif
}

#ifdef TEST
# include <stdio.h>
# include <unistd.h>

# include "cmdtmio.h"
# include "error.h"
# include "exit.h"

char *program_name = "mktime";

static void
usage (int status)
{
  printusage ("mktime", " [\"" DST_NAME "\"|\"" ST_NAME "\"]\n\
       [-]YEAR [-]MONTH [-]DAY [-]HOUR [-]MINUTE [-]SECOND\n\
Convert YEAR, MONTH, DAY, HOUR, MINUTES, and SECOND into seconds\n\
elapsed since 1970-01-01 00:00 UTC and adjust each parameter of time\n\
to the range of correct values. Display adjusted time if conversion\n\
and adjustment is performed, othewise, \"-0001-00-00 00:00:00\".\
", true, true, 'T');
  fputs ("\
\n\
Options:\n\
  -a   output time with week day name and time zone\n\
  -I   output time in ISO 8601 format\n\
  -J   output time in Japanese era name and number\n\
  -s   output time " IN_UNIX_SECONDS "\n\
  -w   output time with week day name\n\
  -W   output time with week number and day\n\
  -Y   output time with year day\n\
  -z   output time with time zone\n\
", stdout);
  exit (status);
}

int
main (int argc, char **argv)
{
  TM tm = (TM) { .tm_mday = 1, .tm_wday = -1, .tm_yday = -1, .tm_isdst = -1 };
  int *dates[] = { &tm.tm_year, &tm.tm_mon, &tm.tm_mday };
  int *times[] = { &tm.tm_hour, &tm.tm_min, &tm.tm_sec };
  intmax_t seconds;
  int c;
  int status = EXIT_FAILURE;
  int set_num;
  char *endptr;
  bool seconds_output = false;
  bool isdst_output = false;
  struct tm_fmt tm_fmt = { false };
  struct tm_ptrs tm_ptrs = (struct tm_ptrs) { .dates = dates, .times = times };

  while ((c = getopt (argc, argv, ":aIJswWYzT")) != -1)
    {
      switch (c)
        {
        case 'a':
          tm_fmt.weekday_name = tm_fmt.no_newline = isdst_output = true;
          tm_ptrs.weekday = &tm.tm_wday;
          tm_ptrs.utcoff = &tm.tm_gmtoff;
          break;
        case 'I':
          tm_fmt.iso8601 = true;
          break;
        case 'J':
          tm_fmt.japanese = true;
          break;
        case 's':
          seconds_output = true;
          break;
        case 'w':
          tm_fmt.weekday_name = true;
          tm_ptrs.weekday = &tm.tm_wday;
          break;
        case 'W':
          tm_fmt.week_numbering = true;
          tm_ptrs.weekday = &tm.tm_wday;
        case 'Y':
          tm_ptrs.yearday = &tm.tm_yday;
          break;
        case 'z':
          tm_fmt.no_newline = isdst_output = true;
          tm_ptrs.utcoff = &tm.tm_gmtoff;
          break;
# ifdef USE_TM_SELFIMPL
        case 'T':
          trans_isdst = 0;
          break;
# endif
        default:
          usage (EXIT_FAILURE);
        }
    }

  argc -= optind;
  argv += optind;

  /* Attempt to set the isdst flag from the first argument. */
  if (argc > 0
      && sscanisdst (*argv, &tm.tm_isdst, &endptr) && *endptr == '\0')
    {
      argc--;
      argv++;
    }

  if (argc <= 0 || argc > 6)
    usage (EXIT_FAILURE);

  /* Set each parameter of struct tm from the rest of arguments
     but only the year must be specified. */
  set_num = sscanreltm (argc, (const char **)argv, &tm_ptrs, &endptr);
  if (set_num < 0)
    error (EXIT_FAILURE, 0, "invalid time value %s", endptr);
  else if (set_num == 0 || *endptr != '\0')
    usage (EXIT_FAILURE);
  else if (set_num >= 2)
    tm.tm_mon--;

  tm.tm_year -= TM_YEAR_BASE;

  seconds = mktimew (&tm);

  if (tm.tm_wday >= 0)
    {
      tm.tm_year += TM_YEAR_BASE;
      tm.tm_mon++;

      status = EXIT_SUCCESS;
    }
  else
    tm = (TM) { .tm_year = -1, .tm_wday = -1, .tm_yday = -1 };

  if (seconds_output)
    printelapse (false, seconds, -1);
  else
    {
      printtm (&tm_fmt, &tm_ptrs);

      if (isdst_output)
        printisdst (false, tm.tm_isdst);
    }

  return status;
}
#endif
