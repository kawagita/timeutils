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

#include "ftsec.h"
#include "wintm.h"

#ifdef USE_TM_SELFIMPL
# include "adjusttm.h"
# include "adjusttz.h"
# include "intoverflow.h"
#endif

/* Convert the specified seconds since 1970-01-01 00:00 UTC to local time
   and set those parameters of time into *TM. Return the pointer to it
   if conversion is performed, otherwise, NULL. */

TM *
localtimew (const intmax_t *seconds, TM *tm)
{
#ifndef USE_TM_SELFIMPL
  time_t t;
# ifdef USE_TM_MSVCRT
  struct tm gmt, lct;
# endif
#endif

  if (secoverflow (*seconds, 0))
    return NULL;

#ifndef USE_TM_SELFIMPL
  t = *seconds;

# ifdef USE_TM_GLIBC
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
#else  /* USE_TM_SELFIMPL */
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

          date.tm_wday = WEEKDAY_FROM (date.tm_wday, adj_day);
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

# include "cmdtmio.h"
# include "error.h"
# include "exit.h"

char *program_name = "localtime";

static void
usage (int status)
{
  printusage ("localtime", " [-]SECONDS[.nnnnnnn]\n\
Convert SECONDS since 1970-01-01 00:00 UTC into parameters of time\n\
in local time zone. Display those time if conversion is performed,\n\
otherwise, \"-0001-00-00 00:00:00\".\n\
\n\
Options:\n\
  -a   output time with week day name, time zone, and \"DST\" or \"ST\"\n\
  -d   output time with \"DST\" or \"ST\"\n\
  -I   output time in ISO 8601 format\n\
  -J   output date in Japanese era name and number\n\
  -w   output time with week day name\n\
  -W   output time with week number and day\n\
  -Y   output time with year day\n\
  -z   output time with time zone\
", true, false, 0);
  exit (status);
}

int
main (int argc, char **argv)
{
  TM tm = (TM) { .tm_year = -1, .tm_wday = -1, .tm_yday = -1, .tm_isdst = -1 };
  int *dates[] = { &tm.tm_year, &tm.tm_mon, &tm.tm_mday };
  int *times[] = { &tm.tm_hour, &tm.tm_min, &tm.tm_sec };
  intmax_t seconds;
  int nsec = 0;
  int c;
  int status = EXIT_FAILURE;
  bool isdst_output = false;
  struct tm_fmt tm_fmt = { false };
  struct tm_ptrs tm_ptrs = (struct tm_ptrs) { .dates = dates, .times = times };

  while ((c = getopt (argc, argv, ":adIJwWYz")) != -1)
    {
      switch (c)
        {
        case 'a':
          tm_fmt.weekday_name = true;
          tm_ptrs.weekday = &tm.tm_wday;
          tm_ptrs.utcoff = &tm.tm_gmtoff;
        case 'd':
          tm_fmt.no_newline = true;
          isdst_output = true;
          break;
        case 'I':
          tm_fmt.iso8601 = true;
          break;
        case 'J':
          tm_fmt.japanese = true;
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
          tm_ptrs.utcoff = &tm.tm_gmtoff;
          break;
        default:
          usage (EXIT_FAILURE);
        }
    }

  argv += optind;

  if (argc <= optind || argc - 1 > optind)
    usage (EXIT_FAILURE);

  /* Set the argument into seconds and its fractional part. */
  char *endptr;
  int set_num = sscanseconds (*argv, &seconds, &nsec, &endptr);
  if (set_num < 0)
    error (EXIT_FAILURE, 0, "invalid seconds %s", *argv);
  else if (set_num == 0 || *endptr != '\0')
    usage (EXIT_FAILURE);

  if (localtimew (&seconds, &tm))
    {
      if (set_num >= 2)
        tm_ptrs.frac_val = &nsec;

      tm.tm_year += TM_YEAR_BASE;
      tm.tm_mon++;

      status = EXIT_SUCCESS;
    }

  printtm (&tm_fmt, &tm_ptrs);

  if (isdst_output)
    printisdst (false, tm.tm_isdst);

  return status;
}
#endif
