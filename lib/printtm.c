/* Output parameters of time to standard output
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

#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "adjusttm.h"
#include "printtm.h"

/* Abbreviations for the week day  */
static const char *wday_abbrs[] =
{
  "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};

/* The abbreviation for unknown week day  */
#define UNKNOWN_WDAY_ABBR "???";

/* Return the week day at January 1st in a year  */
#define YEAR_1ST_WEEKDAY(yday,wday) (((wday) - ((yday) % 7) + 7) % 7)

/* Return the week day in ISO 8601  */
#define ISO8601_WEEKDAY(wday) ((wday) > 0 ? (wday) : 7)

/* Return the number of ISO 8601 weeks in the specified year  */
#define ISO8601_WEEKS(year,y1st_wday,has_noleapday) \
  (((y1st_wday) ^ 4 && ((y1st_wday) ^ 3 || (has_noleapday))) ? 52 : 53)

/* Return the week number for the specified day in *YEAR.  */
int
weeknumber (int *year, int yday, int wday, bool iso8601)
{
  int weeknum = 1;

  if (wday < 0)
    return 0;
  else if (iso8601)
    {
      int y1st_wday = YEAR_1ST_WEEKDAY (yday, wday);
      int w = (11 + yday - ISO8601_WEEKDAY (wday)) / 7;
      if (w < 1)
        {
          int last_year = *year - 1;
          bool has_noleapday = HAS_NOLEAPDAY (last_year);
          int last_ydays = has_noleapday ? DAYS_IN_YEAR : DAYS_IN_LEAPYEAR;

          /* Calculate the number of weeks in the last year if the specified
             day is not included in the first week. */
          weeknum = ISO8601_WEEKS (last_year,
                      YEAR_1ST_WEEKDAY (last_ydays - 1, (y1st_wday + 6) % 7),
                      has_noleapday);
          *year = last_year;
        }
      else if (w > ISO8601_WEEKS (*year, y1st_wday, HAS_NOLEAPDAY (*year)))
        (*year)++;
      else
        weeknum = w;
    }
  else
    {
      int ydays_since_w2 = yday - wday;
      if (ydays_since_w2 > 0)
        weeknum += ydays_since_w2 / 7 + (ydays_since_w2 % 7 > 0 ? 1 : 0);
    }

  return weeknum;
}

/* The property of era  */
struct era_prop
{
  int symbol;
  int start_number;
  int from_year;
  int from_yday;
  int to_year;
  int to_yday;
};

/* Properties of japanese era  */
static struct era_prop jera_props[] =
{
  { 'M', 6, 1873,   0, 1912, 210 },
  { 'T', 1, 1912, 211, 1926, 357 },
  { 'S', 1, 1926, 358, 1989,   6 },
  { 'H', 1, 1989,   7, 2019, 119 },
  { 'R', 1, 2019, 120, INT_MAX, INT_MAX },
  { 0 }
};

/* Return a symbol character for the japanese era in YEAR and YDAY.  */
int
japanese_era (int *year, int yday)
{
  struct era_prop *jera_prop = jera_props;
  if (jera_prop->from_year <= *year)
    {
      while (jera_prop->symbol)
        {
          if ((jera_prop->from_year == *year && jera_prop->from_yday <= yday)
              || (jera_prop->from_year < *year && jera_prop->to_year > *year)
              || (jera_prop->to_year == *year && jera_prop->to_yday >= yday))
            {
              *year -= jera_prop->from_year - jera_prop->start_number;

              return jera_prop->symbol;
            }

          jera_prop++;
        }
    }

  return 0;
}

/* Return the string of a state whether DST is in effect  */
#define DST_STATE(isdst) ((isdst) ? ((isdst) > 0 ? " DST" : " N/A") : "")

/* A delimiter between seconds since Unix epoch and parameters of time  */
#define TM_DELIM "\t"

/* Output each parameter of time included in *TM if its pointer is not NULL,
   according to *TM_FMT. Return the number of output parameters.  */

int
printtm (const struct tmout_ptrs *tm_ptrs, const struct tmout_fmt *tm_fmt)
{
  int tmout_size = 0;
  bool elapse_leading = false;
  bool hour_output = false;
  bool sec_output = false;
  bool japanese = tm_fmt->japanese && ((tm_ptrs->tm_mon && tm_ptrs->tm_mday)
                                       || tm_ptrs->tm_yday);
  bool iso8601 = tm_fmt->iso8601 && !tm_fmt->japanese;
  bool week_numbering = tm_fmt->week_numbering && !tm_fmt->japanese
                        && tm_ptrs->tm_wday && tm_ptrs->tm_yday;

  /* Output seconds since Unix epoch. */
  if (tm_ptrs->tm_elapse)
    {
#if defined USE_TM_CYGWIN || _WIN32_WINNT >= 0x600
      printf ("%jd", *(tm_ptrs->tm_elapse));
#else
      printf ("%I64d", *(tm_ptrs->tm_elapse));
#endif
      tmout_size++;
      elapse_leading = true;
      sec_output = true;
    }

  /* Output an abbreviation for the week day. */
  if (tm_ptrs->tm_wday && tm_fmt->weekday_name)
    {
      const char *wday_abbr = UNKNOWN_WDAY_ABBR;
      int wday = *(tm_ptrs->tm_wday);
      if (wday >= 0 && wday <= 6)
        wday_abbr = wday_abbrs[wday];

      if (elapse_leading)
        {
          fputs (TM_DELIM, stdout);
          elapse_leading = false;
        }
      printf ("%s", wday_abbr);
      tmout_size++;
    }

  /* Output the date, starting with the year. */
  if (tm_ptrs->tm_year)
    {
      int year = *(tm_ptrs->tm_year) + TM_YEAR_BASE;
      int date_delim = '-';
      int era_symbol = 0;
      int weeknum = -1;

      if (elapse_leading)
        {
          fputs (TM_DELIM, stdout);
          elapse_leading = false;
        }
      else if (tmout_size > 0)
        fputs (" ", stdout);

      /* Calculate the era symbol or week number of the specified date firstly
         because year changes may vary. */
      if (japanese)
        {
          int yday = tm_ptrs->tm_yday ? *(tm_ptrs->tm_yday)
                                      : YEAR_DAYS (year, *(tm_ptrs->tm_mon))
                                        + *(tm_ptrs->tm_mday) - 1;
          era_symbol = japanese_era (&year, yday);
        }
      else if (week_numbering)
        weeknum = weeknumber (&year,
                    *(tm_ptrs->tm_yday), *(tm_ptrs->tm_wday), iso8601);

      if (era_symbol)
        {
          printf ("%c%02d", era_symbol, year);
          date_delim = '.';
        }
      else if (year >= 0)
        printf ("%04d", year);
      else
        printf ("%05d", year);
      tmout_size++;

      if (weeknum >= 0)  /* Week number and day */
        {
          printf ("-W%02d", weeknum);
          tmout_size++;

          if (!tm_fmt->weekday_name)
            {
              int wday = *(tm_ptrs->tm_wday);
              if (wday < 0)
                wday = 0;
              else if (iso8601)
                wday = ISO8601_WEEKDAY (wday);

              printf ("-%d", wday);
              tmout_size++;
            }
        }
      else if (tm_ptrs->tm_yday && !era_symbol)  /* Ordinal date */
        {
          printf ("-%03d", *(tm_ptrs->tm_yday) + 1);
          tmout_size++;
        }
      else if (tm_ptrs->tm_mon)  /* Month and day */
        {
          printf ("%c%02d", date_delim, *(tm_ptrs->tm_mon) + 1);
          tmout_size++;

          if (tm_ptrs->tm_mday)
            {
              printf ("%c%02d", date_delim, *(tm_ptrs->tm_mday));
              tmout_size++;
            }
        }
    }

  /* Output the time in a day, starting with the hour. */
  if (tm_ptrs->tm_hour)
    {
      if (elapse_leading)
        {
          fputs (TM_DELIM, stdout);
          elapse_leading = false;
        }
      else if (tm_ptrs->tm_year && iso8601)
        fputs ("T", stdout);
      else if (tmout_size > 0)
        fputs (" ", stdout);
      printf ("%02d", *(tm_ptrs->tm_hour));
      tmout_size++;
      hour_output = true;

      if (tm_ptrs->tm_min)  /* Minute and second */
        {
          printf (":%02d", *(tm_ptrs->tm_min));
          tmout_size++;

          if (tm_ptrs->tm_sec)
            {
              printf (":%02d", *(tm_ptrs->tm_sec));
              tmout_size++;
              sec_output = true;
            }
        }
    }

  /* Output the fractional part of seconds. */
  if (tm_ptrs->tm_frac && sec_output)
    {
      printf (".%07d", *(tm_ptrs->tm_frac));
      tmout_size++;
    }

  /* Output the information of time zone. */
  if (tm_ptrs->tm_gmtoff && hour_output)
    {
      long int gmtoff_min = *(tm_ptrs->tm_gmtoff) / 60;
      long int abs_gmtoff_min = gmtoff_min < 0 ? - gmtoff_min : gmtoff_min;

      if (!iso8601)
        fputs (" ", stdout);
      printf ("%c%02ld%02d", (gmtoff_min < 0 ? '-' : '+'),
              abs_gmtoff_min / 60, (int)(abs_gmtoff_min % 60));
      tmout_size++;

      if (tm_ptrs->tm_isdst && !iso8601)  /* DST or not */
        {
          fputs (DST_STATE (*(tm_ptrs->tm_isdst)), stdout);
          tmout_size++;
        }
    }

  /* Output the trailing newline if "-n" is not specified by the command. */
  if (!tm_fmt->no_newline && tmout_size > 0)
    fputs ("\n", stdout);

  return tmout_size;
}
