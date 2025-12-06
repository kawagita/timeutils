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
#include "cmdtmio.h"
#include "ftsec.h"

/* Abbreviations for the week day  */
static const char *wday_abbrs[] =
{
  "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};

/* The abbreviation for unknown week day  */
#define UNKNOWN_WDAY_ABBR "???";

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
      int y1st_wday = WEEKDAY_FROM (wday, - yday);  /* Weed day of 1 Jan */
      int w = (11 + yday - ISO8601_WEEKDAY (wday)) / 7;
      if (w < 1)
        {
          if (*year <= INT_MIN)
            return 0;

          int last_year = *year - 1;
          bool has_noleapday = HAS_NOLEAPDAY (last_year);
          int last_ydays = has_noleapday ? DAYS_IN_YEAR : DAYS_IN_LEAPYEAR;

          /* Calculate the number of weeks in the last year if the specified
             day is not included in the first week. */
          weeknum = ISO8601_WEEKS (last_year,
                      WEEKDAY_FROM ((y1st_wday + 6) % 7, 1 - last_ydays),
                      has_noleapday);
          *year = last_year;
        }
      else if (w > ISO8601_WEEKS (*year, y1st_wday, HAS_NOLEAPDAY (*year)))
        {
          if (*year < INT_MAX)
            (*year)++;
          else
            /* Return the maximum week number 53 + 1 if overflow. */
            weeknum = 54;
        }
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

/* Output a parameter of date to standard output.  */
static int
printdate (int value, int width, int delim)
{
  char format[] = "%01d";

  if (value < 0 && width == 2)
    {
      /* Change the delimiter to '+' if negative value. */
      value = value > INT_MIN ? - value : INT_MAX;
      delim = '+';
    }

  format[2] = '0' + width;

  if (delim)
    fputc (delim, stdout);

  printf (format, value);

  return 1;
}

/* Output parameters of date or time included in *TM_PTRS to standard
   output, according to the format defined for each flag set in *TM_FMT.
   Return the number of output parameters.  */

int
printtm (const struct tm_fmt *tm_fmt, const struct tm_ptrs *tm_ptrs)
{
  bool week_numbering = tm_fmt->week_numbering
                        && tm_ptrs->weekday && tm_ptrs->yearday;
  bool japanese = tm_fmt->japanese && (tm_ptrs->dates || tm_ptrs->yearday);
  bool iso8601 = tm_fmt->iso8601 && !japanese;
  int out_num = 0;
  int i;

  /* Output an abbreviation for the week day. */
  if (tm_ptrs->weekday && tm_fmt->weekday_name)
    {
      const char *wday_abbr = UNKNOWN_WDAY_ABBR;
      int wday = *tm_ptrs->weekday;
      if (wday >= 0 && wday <= 6)
        wday_abbr = wday_abbrs[wday];

      printf ("%s", wday_abbr);
      out_num++;

      if (tm_ptrs->weekday_ordinal)
        printf (",%" PRIdMAX, *tm_ptrs->weekday_ordinal);
    }

  /* Output the date, calculated from the year, month, and day. */
  if (tm_ptrs->dates)
    {
      int year = *tm_ptrs->dates[0];
      int year_width = year < 0 ? 5 : 4;
      int date_delim = '-';
      int weeknum = -1;
      int yeardaynum = tm_ptrs->yearday ? *tm_ptrs->yearday + 1 : -1;

      if (out_num > 0)
        fputc (' ', stdout);

      /* Calculate the era symbol or week number of the specified date
         firstly because year changes may vary. */
      if (week_numbering)
        weeknum = weeknumber (&year,
                    *tm_ptrs->yearday, *tm_ptrs->weekday, iso8601);

      if (japanese)
        {
          int yday = yeardaynum > 0 ? yeardaynum - 1
                                    : YEAR_DAYS (year, *tm_ptrs->dates[1])
                                    + *tm_ptrs->dates[2] - 1;
          int era_symbol = japanese_era (&year, &yday);
          if (era_symbol)
            {
              fputc (era_symbol, stdout);

              year_width = 2;
              date_delim = '.';

              if (yeardaynum > 0)
                yeardaynum = yday + 1;
            }
        }

      out_num += printdate (year, year_width, 0);

      if (weeknum >= 0)  /* Week date */
        {
          printf ("-W%02d", weeknum);
          out_num++;

          if (!tm_fmt->weekday_name)
            {
              int wday = *tm_ptrs->weekday;
              if (wday < 0)
                wday = 0;
              else if (iso8601)
                wday = ISO8601_WEEKDAY (wday);

              printf ("-%d", wday);
              out_num++;
            }
        }
      else if (yeardaynum >= 0)  /* Ordinal date */
        {
          printf ("-%03d", yeardaynum);
          out_num++;
        }
      else  /* Calendar date */
        {
          for (i = 1; i < 3; i++)
            out_num += printdate (*tm_ptrs->dates[i], 2, date_delim);
        }
    }

  /* Output the hour, minute, and second. */
  if (tm_ptrs->times)
    {
      if (iso8601)
        fputc ('T', stdout);
      else if (out_num > 0)
        fputc (' ', stdout);

      for (i = 0; i < 3; i++)
        {
          if (i > 0)
            fputc (':', stdout);

          printf ("%02d", *tm_ptrs->times[i]);
          out_num++;
        }

      /* Output the nanoseconds less than a second. */
      if (tm_ptrs->ns)
        {
          printf (FT_NSEC_FORMAT, *tm_ptrs->ns);
          out_num++;
        }
    }

  /* Output the UTC offset in a time zone. */
  if (tm_ptrs->utcoff)
    {
      long int utcoff_min = *tm_ptrs->utcoff / 60;
      long int abs_utcoff_min = utcoff_min;
      if (utcoff_min == LONG_MIN)
        abs_utcoff_min = LONG_MAX;
      else if (utcoff_min < 0)
        abs_utcoff_min = - utcoff_min;

      if (iso8601)
        {
          if (out_num == 0)
            fputc ('Z', stdout);
        }
      else if (out_num > 0)
        fputc (' ', stdout);

      printf ("%c%02ld%02d", (utcoff_min < 0 ? '-' : '+'),
              abs_utcoff_min / 60, (int)(abs_utcoff_min % 60));
      out_num++;
    }

  if (!tm_fmt->no_newline && out_num > 0)
    fputc ('\n', stdout);

  return out_num;
}
