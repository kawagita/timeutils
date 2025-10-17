/* Get the local time in NTFS
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

#include "config.h"

#include <windows.h>
#include <stdbool.h>
#include <stdint.h>

#include "intoverflow.h"
#include "timeoverflow.h"
#include "wintm.h"

#ifndef USE_TM_WRAPPER
# include <limits.h>
# include <time.h>

# ifdef USE_TM_MSVCRT
long int tm_diff (struct tm const *a, struct tm const *b);
# endif
#else  /* USE_TM_WRAPPER */
# include "adjusttm.h"
# include "adjusttz.h"
#endif

/* Convert the specified seconds since Unix epoch to local time and set
   parameters of time into *TM. Return the pointer to it if conversion is
   performed, otherwise, NULL.  */

TM *
localtimew (const intmax_t *seconds, TM *tm)
{
#ifndef USE_TM_WRAPPER
  time_t t;
# ifdef USE_TM_MSVCRT
  struct tm gmt, lct;
# endif
#endif

  if (timew_overflow (*seconds))
    return NULL;

#ifndef USE_TM_WRAPPER
  t = *seconds;

# ifdef USE_TM_CYGWIN
  if (! localtime_r (&t, tm))
    return NULL;
# else  /* USE_TM_MSVCRT */
  if (gmtime_s (&gmt, &t) || gmt.tm_wday < 0
      || localtime_s (&lct, &t) || lct.tm_wday < 0)
    return NULL;

  tm->tm_year = lct.tm_year;
  tm->tm_mon = lct.tm_mon;
  tm->tm_mday = lct.tm_mday;
  tm->tm_hour = lct.tm_hour;
  tm->tm_min = lct.tm_min;
  tm->tm_sec = lct.tm_sec;
  tm->tm_wday = lct.tm_wday;
  tm->tm_yday = lct.tm_yday;
  tm->tm_isdst = lct.tm_isdst;
  tm->tm_gmtoff = tm_diff (&lct, &gmt);
# endif
#else  /* USE_TM_WRAPPER */
  TIME_ZONE_INFORMATION tzinfo;
  struct dtm date;
  struct lctm lct;
  int days = *seconds / SECONDS_IN_DAY;
  int hour = 0;
  int min = 0;
  int sec = 0;

  GetTimeZoneInformation (&tzinfo);

  date.tm_year = UNIXEPOCH_YEAR - TM_YEAR_BASE;
  date.tm_mon = 0;
  date.tm_mday = days + 1;
  date.tm_wday = -1;

  sec = *seconds - days * SECONDS_IN_DAY;

  /* Add minutes (- tzinfo.Bias) by the offset of time zone to parameters
     of time converted from UTC seconds since Unix epoch and adjust those
     to the range of correct values. */
  if (tzinfo.Bias > INT_MAX || tzinfo.Bias < INT_MIN
      || INT_SUBTRACT_WRAPV (min, tzinfo.Bias, &min)
      || ! carrytm (&min, &sec, 60) || ! carrytm (&hour, &min, 60)
      || ! carrytm (&date.tm_mday, &hour, 24) || ! adjustday (&date))
    return NULL;

  /* Adjust parameters of time for the increase or decrease of minutes by
     the DST offset of time zone to the range of correct values. */
  lct.tm_year = date.tm_year;
  lct.tm_ysec = date.tm_yday * SECONDS_IN_DAY + SECONDS_AT (hour, min, sec);
  lct.tm_min = min;
  lct.tm_isdst = -1;

  if (! adjusttz (&lct, -1))
    return NULL;

  if (min ^ lct.tm_min)
    {
      int adj_day = 0;

      min = lct.tm_min;
      if (! carrytm (&hour, &min, 60) || ! carrytm (&adj_day, &hour, 24))
        return NULL;

      if (adj_day)
        {
          if (INT_ADD_WRAPV (date.tm_mday, adj_day, &date.tm_mday)
              || ! adjustday (&date))
            return NULL;

          date.tm_wday = ((adj_day % 7) + date.tm_wday + 7) % 7;
        }
    }

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
#endif

  return tm;
}

#ifdef TEST
# include <stdio.h>
# include <unistd.h>

# include "error.h"
# include "exit.h"
# include "imaxoverflow.h"
# include "printtm.h"
# include "sscantm.h"

char *program_name = "localtime";

static void
usage (int status)
{
  fputs ("Usage: localtime [OPTION]... [-]SECONDS[.nnnnnnn]\n", stdout);
  fputs ("\
Convert SECONDS since 1970-01-01 00:00 UTC into parameters of time\n\
in local timezone. Display those time if conversion is performed,\n\
otherwise, \"0000-00-00 00:00:00\".\n\
\n\
Options:\n\
  -a   output time with week day name and time zone\n\
  -I   output time in ISO 8601 format\n\
  -J   output date in Japanese era name and number\n\
  -n   don't output the trailing newline\n\
  -w   output time with week day name\n\
  -W   output time with week number and day\n\
  -Y   output time with year day\n\
  -z   output time with time zone\n\
", stdout);
  exit (status);
}

static const struct tmimax_prop sec_props[] =
{
  { 0, INTMAX_MIN, INTMAX_MAX, 0, '.' },
  { 0, INTMAX_MIN, INTMAX_MAX, FILETIME_FRAC_DIGITS, '\0' },
};

int
main (int argc, char **argv)
{
  struct tmout_fmt tm_fmt = { false };
  struct tmout_ptrs tm_ptrs = { NULL };
  TM tm;
  intmax_t sec_values[2];
  intmax_t *sec_valp[] = { &sec_values[0], &sec_values[1] };
  int nsec = -1;
  int c;
  int status = EXIT_FAILURE;
  int set_num;
  char *endptr;

  tm_ptrs.tm_year = &tm.tm_year;
  tm_ptrs.tm_mon = &tm.tm_mon;
  tm_ptrs.tm_mday = &tm.tm_mday;
  tm_ptrs.tm_hour = &tm.tm_hour;
  tm_ptrs.tm_min = &tm.tm_min;
  tm_ptrs.tm_sec = &tm.tm_sec;

  tm.tm_year = - TM_YEAR_BASE;
  tm.tm_mon = -1;
  tm.tm_mday = 0;
  tm.tm_wday = tm.tm_yday = -1;
  tm.tm_hour = tm.tm_min = tm.tm_sec = 0;
  tm.tm_isdst = -1;
  tm.tm_gmtoff = 0;

  while ((c = getopt (argc, argv, ":aIJnwWYz")) != -1)
    {
      switch (c)
        {
        case 'a':
          tm_fmt.weekday_name = true;
          tm_ptrs.tm_wday = &tm.tm_wday;
          tm_ptrs.tm_gmtoff = &tm.tm_gmtoff;
          tm_ptrs.tm_isdst = &tm.tm_isdst;
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
          tm_ptrs.tm_wday = &tm.tm_wday;
          break;
        case 'W':
          tm_fmt.week_numbering = true;
          tm_ptrs.tm_wday = &tm.tm_wday;
        case 'Y':
          tm_ptrs.tm_yday = &tm.tm_yday;
          break;
        case 'z':
          tm_ptrs.tm_gmtoff = &tm.tm_gmtoff;
          tm_ptrs.tm_isdst = &tm.tm_isdst;
          break;
        default:
          usage (EXIT_FAILURE);
        }
    }

  argv += optind;

  if (argc <= optind || argc - 1 > optind)
    usage (EXIT_FAILURE);

  set_num = sscantmimaxp (*argv, sec_props, sec_valp, &endptr);
  if (set_num < 0
      || (set_num > 1 && INT_ADD_WRAPV (0, sec_values[1], &nsec)))
    error (EXIT_FAILURE, 0, "invalid seconds %s", *argv);
  else if (set_num == 0 || *endptr != '\0')
    usage (EXIT_FAILURE);

  if (localtimew (&sec_values[0], &tm))
    {
      if (nsec >= 0)
        tm_ptrs.tm_frac = &nsec;

      status = EXIT_SUCCESS;
    }

  printtm (&tm_fmt, &tm_ptrs);

  return status;
}
#endif
