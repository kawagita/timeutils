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

#include <inttypes.h>
#include <limits.h>
#include <stdbool.h>
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
static int
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
static int
japanese_era (int *year, int *yday)
{
  struct era_prop *jera_prop = jera_props;
  if (jera_prop->from_year <= *year)
    {
      while (jera_prop->symbol)
        {
          if ((jera_prop->from_year == *year && jera_prop->from_yday <= *yday)
              || (jera_prop->from_year < *year && jera_prop->to_year > *year)
              || (jera_prop->to_year == *year && jera_prop->to_yday >= *yday))
            {
              if (jera_prop->from_year == *year)
                *yday -= jera_prop->from_yday;
              *year -= jera_prop->from_year - jera_prop->start_number;

              return jera_prop->symbol;
            }

          jera_prop++;
        }
    }

  return 0;
}

/* Output a parameter of date or time to standard output.  */
static int
printdatetime (int value, int leading_char, int width)
{
  char format[] = "%01d";

  if (width > 0 && width < 10)
    {
      if (value < 0 && width == 2)
        {
          /* Change the delimiter to '+' if negative value. */
          value = value > INT_MIN ? - value : INT_MAX;
          leading_char = '+';
        }

      format[2] = '0' + width;
    }

  if (leading_char)
    fputc (leading_char, stdout);

  printf (format, value);

  return 1;
}

/* Output each parameter of time included in *TM_PTRS to standard output
   if its pointer is not NULL, according to the format of members in
   *TM_FMT. Return the number of output parameters.  */

int
printtm (const struct tmout_fmt *tm_fmt, const struct tmout_ptrs *tm_ptrs)
{
  int tmout_size = 0;
  bool elapse_leading = false;
  bool hour_output = false;
  bool sec_output = false;
  bool japanese = tm_fmt->japanese && ((tm_ptrs->tm_mon && tm_ptrs->tm_mday)
                                       || tm_ptrs->tm_yday);
  bool iso8601 = tm_fmt->iso8601 && !japanese;
  bool week_numbering = tm_fmt->week_numbering && !japanese
                        && tm_ptrs->tm_wday && tm_ptrs->tm_yday;
  bool relative = tm_fmt->relative && !japanese && !iso8601;
  const char *elapse_delim = tm_ptrs->elapse_delim
                             ? tm_ptrs->elapse_delim : " ";

  /* Output the value elapsed since a time. */
  if (tm_ptrs->tm_elapse)
    {
      printf ("%" PRIdMAX, *tm_ptrs->tm_elapse);
      tmout_size++;
      elapse_leading = true;
      sec_output = tm_ptrs->elapse_delim ? false : true;
    }

  /* Output an abbreviation for the week day. */
  if (tm_ptrs->tm_wday && tm_fmt->weekday_name)
    {
      const char *wday_abbr = UNKNOWN_WDAY_ABBR;
      int wday = *tm_ptrs->tm_wday;
      if (wday >= 0 && wday <= 6)
        wday_abbr = wday_abbrs[wday];

      if (elapse_leading)
        {
          fputs (elapse_delim, stdout);
          elapse_leading = false;
        }

      printf ("%s", wday_abbr);
      tmout_size++;
    }

  /* Output the date, starting with the year. */
  if (tm_ptrs->tm_year)
    {
      int year = *tm_ptrs->tm_year + (relative ? 0 : TM_YEAR_BASE);
      int year_leading_char = 0;
      int date_delim = '-';
      int date_width = 0;
      int weeknum = -1;
      int yday = tm_ptrs->tm_yday ? *tm_ptrs->tm_yday : -1;

      if (elapse_leading)
        {
          fputs (elapse_delim, stdout);
          elapse_leading = false;
        }
      else if (tmout_size > 0)
        year_leading_char = ' ';

      /* Calculate the era symbol or week number of the specified date
         firstly because year changes may vary. */
      if (japanese)
        {
          int era_yday = yday >= 0 ? *tm_ptrs->tm_yday
                                   : YEAR_DAYS (year, *tm_ptrs->tm_mon)
                                     + *tm_ptrs->tm_mday - 1;
          int era_symbol = japanese_era (&year, &era_yday);
          if (era_symbol)
            {
              if (year_leading_char)
                fputc (year_leading_char, stdout);

              year_leading_char = era_symbol;
              date_width = 2;
              date_delim = '.';

              if (yday >= 0)
                yday = era_yday;
            }
        }
      else if (week_numbering)
        weeknum = weeknumber (&year,
                    *tm_ptrs->tm_yday, *tm_ptrs->tm_wday, iso8601);

      if (relative)
        {
          date_delim = ' ';
          date_width = 1;
        }
      else if (date_width == 0)
        date_width = year < 0 ? 5 : 4;

      tmout_size += printdatetime (year, year_leading_char, date_width);

      if (weeknum >= 0)  /* Week number and day */
        {
          printf ("-W%02d", weeknum);
          tmout_size++;

          if (!tm_fmt->weekday_name)
            {
              int wday = *tm_ptrs->tm_wday;
              if (wday < 0)
                wday = 0;
              else if (iso8601)
                wday = ISO8601_WEEKDAY (wday);

              printf ("-%d", wday);
              tmout_size++;
            }
        }
      else if (yday >= 0)  /* Ordinal date */
        {
          printf ("-%03d", yday + 1);
          tmout_size++;
        }
      else if (tm_ptrs->tm_mon)  /* Month and day */
        {
          int month = *tm_ptrs->tm_mon;

          if (!relative)
            {
              month++;
              date_width = 2;
            }

          tmout_size += printdatetime (month, date_delim, date_width);

          if (tm_ptrs->tm_mday)
            tmout_size += printdatetime (
                            *tm_ptrs->tm_mday, date_delim, date_width);
        }
    }

  /* Output the time in a day, starting with the hour. */
  if (tm_ptrs->tm_hour)
    {
      int hour = *tm_ptrs->tm_hour;
      int hour_leading_char = 0;
      int time_delim = ':';
      int time_width = 2;

      if (elapse_leading)
        {
          fputs (elapse_delim, stdout);
          elapse_leading = false;
        }
      else if (tm_ptrs->tm_year && iso8601)
        hour_leading_char = 'T';
      else if (tmout_size > 0)
        hour_leading_char = ' ';

      if (relative)
        {
          time_delim = ' ';
          time_width = 1;
        }

      tmout_size += printdatetime (hour, hour_leading_char, time_width);
      hour_output = true;

      if (tm_ptrs->tm_min)  /* Minute and second */
        {
          tmout_size += printdatetime (
                          *tm_ptrs->tm_min, time_delim, time_width);

          if (tm_ptrs->tm_sec)
            {
              tmout_size += printdatetime (
                              *tm_ptrs->tm_sec, time_delim, time_width);
              sec_output = true;
            }
        }
    }

  /* Output the fractional part of seconds. */
  if (tm_ptrs->tm_frac && sec_output)
    {
      int frac_delim = '.';
      int frac_width = 7;

      if (relative)
        {
          frac_delim = ' ';
          frac_width = 1;
        }

      tmout_size += printdatetime (*tm_ptrs->tm_frac, frac_delim, frac_width);
    }

  /* Output the information of time zone. */
  if (tm_ptrs->tm_gmtoff && hour_output)
    {
      long int gmtoff_min = *tm_ptrs->tm_gmtoff / 60;
      long int abs_gmtoff_min = gmtoff_min < 0 ? - gmtoff_min : gmtoff_min;

      if (!iso8601)
        fputc (' ', stdout);

      printf ("%c%02ld%02d", (gmtoff_min < 0 ? '-' : '+'),
              abs_gmtoff_min / 60, (int)(abs_gmtoff_min % 60));
      tmout_size++;

      if (tm_ptrs->tm_isdst && !iso8601)  /* DST or not */
        {
          char *dst_state = "";
          int isdst = *tm_ptrs->tm_isdst;
          if (isdst > 0)
            dst_state = " DST";
          else if (isdst < 0)
            dst_state = " UNK";

          fputs (dst_state, stdout);
          tmout_size++;
        }
    }

  /* Output the trailing newline if "-n" is not specified by the command. */
  if (!tm_fmt->no_newline && tmout_size > 0)
    fputc ('\n', stdout);

  return tmout_size;
}
