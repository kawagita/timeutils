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

#include <windows.h>
#include <stdbool.h>
#include <stdint.h>

#include "wintm.h"

#ifndef USE_TM_WRAPPER
# include <time.h>

# ifdef USE_TM_MSVCRT
long int tm_diff (struct tm const *a, struct tm const *b);
# endif
#else  /* USE_TM_WRAPPER */
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
#ifndef USE_TM_WRAPPER
# ifdef USE_TM_CYGWIN
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
#else  /* USE_TM_WRAPPER */
  struct dtm date;
  struct lctm lct;
  intmax_t seconds;
  intmax_t epochdays;
  intmax_t offset = 0;
  int epochyears;
  int year;
  int ldays = 0;
  int hour = tm->tm_hour;
  int min = tm->tm_min;
  int sec = tm->tm_sec;
  int adj_min;

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

  /* Count the number of days since Unix epoch including some leap days
     and set the week day to the remainder of dividing its value by 7. */
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

  if (IMAX_MULTIPLY_WRAPV (epochyears, 365, &epochdays)
      || (ldays && IMAX_ADD_WRAPV (epochdays, ldays, &epochdays))
      || IMAX_ADD_WRAPV (epochdays, date.tm_yday, &epochdays))
    return -1;

  date.tm_wday = ((int)(epochdays % 7) + UNIXEPOCH_WEEKDAY + 7) % 7;

  /* Add the number of seconds converted from time in a day to Unix seconds. */
  if (IMAX_MULTIPLY_WRAPV (epochdays, SECONDS_IN_DAY, &seconds)
      || IMAX_ADD_WRAPV (seconds, SECONDS_AT (hour, min, sec), &seconds)
      || timew_overflow (seconds))
    return -1;

  /* Adjust parameters of time for the increase or decrease of minutes by
     the offset of time zone to the range of correct values. */
  lct.tm_year = date.tm_year;
  lct.tm_ysec = date.tm_yday * SECONDS_IN_DAY + SECONDS_AT (hour, min, sec);
  lct.tm_min = min;
  lct.tm_isdst = tm->tm_isdst;

  if (! adjusttz (&lct, trans_isdst))
    return -1;

  adj_min = lct.tm_min - min;
  if (adj_min)
    {
      int adj_day = 0;

      min = lct.tm_min;
      if (IMAX_MULTIPLY_WRAPV (adj_min, 60, &offset)
          || ! carrytm (&hour, &min, 60) || ! carrytm (&adj_day, &hour, 24))
        return -1;

      if (adj_day)
        {
          if (INT_ADD_WRAPV (date.tm_mday, adj_day, &date.tm_mday)
              || ! adjustday (&date))
            return -1;

          date.tm_wday = ((adj_day % 7) + date.tm_wday + 7) % 7;
        }
    }

  /* Subtract the offset of time zone from UTC seconds since Unix epoch. */
  if (IMAX_SUBTRACT_WRAPV (offset, lct.tm_gmtoff, &offset)
      || (offset && (IMAX_ADD_WRAPV (seconds, offset, &seconds)
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
# include <limits.h>
# include <unistd.h>

# include "error.h"
# include "exit.h"
# include "printtm.h"
# include "sscantm.h"

char *program_name = "mktime";

# ifndef USE_TM_WRAPPER
#  define TRANS_DST_EFFECT_OPTIONS ""
# else
#  define TRANS_DST_EFFECT_OPTIONS " [-C|-M]"
# endif

static void
usage (int status)
{
  printf ("Usage: mktime [OPTION]... [-D|-S]%s\n\
       [-]YEAR [-]MONTH [-]DAY [-]HOUR [-]MINUTE [-]SECOND\n",
          TRANS_DST_EFFECT_OPTIONS);
  fputs ("\
Convert YEAR, MONTH, DAY, HOUR, MINUTES, and SECOND into seconds elapsed\n\
since 1970-01-01 00:00 UTC and adjust each parameter of time to the range\n\
of correct values. Display adjusted time if conversion and adjustment is\n\
performed, othewise, \"0000-00-00 00:00:00\".\n\
\n\
Adjust seconds and time if DST is in effect. With -D or -S, specified\n\
parameters of time have already been effected by DST offset or not.\n\
\n\
", stdout);
# ifdef USE_TM_WRAPPER
  fputs ("\
With -C (default) or -M, adjust time by DST offset or not for a time\n\
that is skipped over and repeated in transition date.\n\
\n\
", stdout);
# endif
  fputs ("\
Options:\n\
  -a   output time with elapsed seconds, week day name, and time zone\n\
  -e   output time with elapsed seconds\n\
  -E   output only elapsed seconds\n\
  -n   don't output the trailing newline\n\
  -I   output time in ISO 8601 format\n\
  -J   output time in Japanese era name and number\n\
  -w   output time with week day name\n\
  -W   output time with week number and day\n\
  -Y   output time with year day\n\
  -z   output time with time zone\n\
", stdout);
  exit (status);
}

static const struct tmint_prop tm_props[] =
{
  { 0, INT_MIN + TM_YEAR_BASE, INT_MAX, 0, '\0' },
  { 0, INT_MIN + 1, INT_MAX, 0, '\0' }
};

int
main (int argc, char **argv)
{
  struct tmout_ptrs tm_ptrs = { NULL };
  struct tmout_fmt tm_fmt = { false };
  TM tm;
  intmax_t seconds;
  int *tm_valp[] =
    {
      &tm.tm_year, &tm.tm_mon, &tm.tm_mday,
      &tm.tm_hour, &tm.tm_min, &tm.tm_sec
    };
  int c, i;
  int status = EXIT_SUCCESS;
  bool only_seconds_set = false;
  bool weekday_set = false;
  bool yearday_set = false;
  bool timezone_set = false;

  tm.tm_mon = tm.tm_mday = 1;
  tm.tm_wday = tm.tm_yday = -1;
  tm.tm_hour = tm.tm_min = tm.tm_sec = 0;
  tm.tm_isdst = -1;
  tm.tm_gmtoff = 0;

  while ((c = getopt (argc, argv, ":aeEIJnwWYzDSCM")) != -1)
    {
      switch (c)
        {
        case 'a':
          tm_ptrs.tm_elapse = &seconds;
          tm_fmt.weekday_name = true;
          weekday_set = true;
          timezone_set = true;
          break;
        case 'E':
          only_seconds_set = true;
        case 'e':
          tm_ptrs.tm_elapse = &seconds;
          break;
        case 'I':
          tm_fmt.iso8601 = true;
          break;
        case 'J':
          tm_fmt.japanese = true;
          break;
        case 'n':
          tm_fmt.no_newline = true;
          break;
        case 'w':
          tm_fmt.weekday_name = true;
          weekday_set = true;
          break;
        case 'W':
          tm_fmt.week_numbering = true;
          weekday_set = true;
        case 'Y':
          yearday_set = true;
          break;
        case 'z':
          timezone_set = true;
          break;
        case 'D':
          tm.tm_isdst = 1;
          break;
        case 'S':
          tm.tm_isdst = 0;
          break;
# ifdef USE_TM_WRAPPER
        case 'C':
          trans_isdst = 1;
          break;
        case 'M':
          trans_isdst = 0;
          break;
# endif
        default:
          usage (EXIT_FAILURE);
        }
    }

  argc -= optind;
  argv += optind;

  if (argc <= 0 || argc > 6)
    usage (EXIT_FAILURE);
  else if (! only_seconds_set)
    {
      tm_ptrs.tm_year = &tm.tm_year;
      tm_ptrs.tm_mon = &tm.tm_mon;
      tm_ptrs.tm_mday = &tm.tm_mday;
      tm_ptrs.tm_hour = &tm.tm_hour;
      tm_ptrs.tm_min = &tm.tm_min;
      tm_ptrs.tm_sec = &tm.tm_sec;
      if (weekday_set)
        tm_ptrs.tm_wday = &tm.tm_wday;
      if (yearday_set)
        tm_ptrs.tm_yday = &tm.tm_yday;
      if (timezone_set)
        {
          tm_ptrs.tm_isdst = &tm.tm_isdst;
          tm_ptrs.tm_gmtoff = &tm.tm_gmtoff;
        }
    }

  /* Set each parameter of time for the value specified to arguments
     but year must be specified. */
  for (i = 0; i < argc; i++)
    {
      char *endptr;
      int set_num;
      if (i < 2)
        set_num = sscantmintp (*argv, &tm_props[i], &tm_valp[i], &endptr);
      else
        set_num = sscantmint (*argv, tm_valp[i], &endptr);
      if (set_num < 0)
        error (EXIT_FAILURE, 0, "invalid time value %s", *argv);
      else if (set_num == 0 || *endptr != '\0')
        usage (EXIT_FAILURE);
      argv++;
    }

  tm.tm_year -= TM_YEAR_BASE;
  tm.tm_mon--;

  seconds = mktimew (&tm);

  if (tm.tm_wday < 0)
    {
      tm.tm_year = - TM_YEAR_BASE;
      tm.tm_mon = -1;
      tm.tm_mday = 0;
      tm.tm_hour = tm.tm_min = tm.tm_sec = 0;

      status = EXIT_FAILURE;
    }

  printtm (&tm_ptrs, &tm_fmt);

  return status;
}
#endif
