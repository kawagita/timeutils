/* Bring the number of day into the range of 0 to the last day in a month
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

#ifndef USE_TM_CYGWIN
# include <windows.h>
#endif
#include <stdbool.h>
#include <stdint.h>

#include "adjusttm.h"
#include "intoverflow.h"

/* Bring the tm_mday member in *TM into the range of correct day in a month
   and adjust other parameters of date by changed days. Calculate and set
   the week day into the tm_wday member when it contains the negative value.
   If adjustment is performed, return true and overwrite *TM by those values,
   otherwise, return false and never change.  */

bool
adjustday (struct dtm *tm)
{
  int year;
  int year0;
  int mon = tm->tm_mon;
  int days;  /* Days from Jan 0th to the correct date */
  int ydays; /* Days from Jan 0th to in the last month in a year */
  bool has_noleapday;

  /* Bring the number of month into the range of 0 to 11 and increase
     or decrease the number of year by its carried value. */
  if (INT_ADD_WRAPV (tm->tm_year, TM_YEAR_BASE, &year)
      || ! carrytm (&year, &mon, 12))
    return false;

  /* Add days from January to the last month in year to the number of days,
     and set the number of month to 0 temporarily. */
  days = YEAR_DAYS (year, mon);
  if (INT_ADD_WRAPV (days, tm->tm_mday, &days))
    return false;

  /* Increase or decrease the number of days in 400 years. */
  if (! adjusttm (&year, 400, &days, DAYS_IN_400YEARS))
    return false;

  /* Convert the number of days into years within 400 years. */
  year0 = year;
  if (! adjusttm (&year, 1, &days, DAYS_IN_YEAR))
    return false;
  else if (year ^ year0)
    {
      int year1 = year;

      if (year1 > year0)
        year1--;
      else
        year0--;

      /* Subtract the number of leap days from days, ignored for the conversion
         to years because of the maximum 97 days less than a year. */
      if (INT_SUBTRACT_WRAPV (days, leapdays (year0, year1), &days))
        return false;
    }

  /* Decrement the number of year to subtract the value from days in the last
     year if the number of days is less than or equal to 0. */
  if (days <= 0)
    {
      if (INT_SUBTRACT_WRAPV (year, 1, &year))
        return false;
      days += YEAR_ALL_DAYS (year);
    }

  /* Cut the value over days of each month in a year from the number of day,
     and set the number of month to the correct value. */
  has_noleapday = HAS_NOLEAPDAY (year);
  mon = 12;
  do
    {
      mon--;
      ydays = yeardays (has_noleapday, mon);
    }
  while (days <= ydays);

  if (INT_SUBTRACT_WRAPV (year, TM_YEAR_BASE, &tm->tm_year))
    return -1;

  tm->tm_mon = mon;
  tm->tm_mday = days - ydays;
  tm->tm_yday = days - 1;

  /* Calculate the week day if the tm_wday member is a negative value. */
  if (tm->tm_wday < 0)
    tm->tm_wday = weekday (year, tm->tm_yday);

  return true;
}

#ifdef TEST
# include <stdio.h>
# include <unistd.h>

# include "cmdtmio.h"
# include "error.h"
# include "exit.h"

char *program_name = "adjustday";

static void
usage (int status)
{
  printusage ("adjustday", " [-]YEAR [-]MONTH [-]DAY\n\
Bring DAY into the range of 0 to the last day in a month and adjust\n\
YEAR and MONTH by carried value. Display those date if adjustment is\n\
performed, othewise, \"-0001-00-00\".\n\
\n\
Options:\n\
  -I   output date in ISO 8601 format\n\
  -J   output date in Japanese era name and number\n\
  -w   output date with week day name\n\
  -W   output date with week number and day\n\
  -Y   output date with year day\
", true, 0);
  exit (status);
}

int
main (int argc, char **argv)
{
  struct dtm tm = (struct dtm) { .tm_mday = 1, .tm_wday = -1, .tm_yday = -1 };
  int *dates[] = { &tm.tm_year, &tm.tm_mon, &tm.tm_mday };
  int c;
  int status = EXIT_FAILURE;
  struct tm_ptrs tm_ptrs = (struct tm_ptrs) { .dates = dates };
  struct tm_fmt tm_fmt = { false };

  while ((c = getopt (argc, argv, ":IJwWY")) != -1)
    {
      switch (c)
        {
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
        default:
          usage (EXIT_FAILURE);
        }
    }

  argc -= optind;
  argv += optind;

  if (argc <= 0 || argc > 3)
    usage (EXIT_FAILURE);

  /* Set each parameter of date for the value specified to arguments
     but year must be specified. */
  char *endptr;
  int set_num = sscanreltm (argc, (const char **)argv, &tm_ptrs, &endptr);
  if (set_num < 0)
    error (EXIT_FAILURE, 0, "invalid date value %s", endptr);
  else if (set_num == 0 || *endptr != '\0')
    usage (EXIT_FAILURE);
  else if (set_num >= 2)
    tm.tm_mon--;

  tm.tm_year -= TM_YEAR_BASE;

  if (adjustday (&tm))
    {
      tm.tm_year += TM_YEAR_BASE;
      tm.tm_mon++;

      status = EXIT_SUCCESS;
    }
  else
    tm = (struct dtm) { .tm_year = -1, .tm_wday = -1, .tm_yday = -1 };

  printtm (&tm_fmt, &tm_ptrs);

  return status;
}
#endif
