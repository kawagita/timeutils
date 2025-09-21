/* adjusttm.h -- Adjustment of time to the range of correct values

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

/* The year of Unix epoch  */

#define UNIXEPOCH_YEAR 1970

/* The week day at Unix epoch  */

#define UNIXEPOCH_WEEKDAY 4

/* The number of days in a year or leap year  */

#define DAYS_IN_YEAR     365
#define DAYS_IN_LEAPYEAR 366

/* The number of days in 4, 100 (not divided by 400), or 400 years,
   including 1, 24, or 97 leap days  */

#define DAYS_IN_4YEARS   1461
#define DAYS_IN_100YEARS 36524
#define DAYS_IN_400YEARS 146097

/* Return true if a leap day is not included in the specified year  */

#define HAS_NOLEAPDAY(year) \
  ((year) & 3 || ((year) % 100 == 0 && (year) % 400 != 0))

/* Return the number of days in the specified year  */

#define YEAR_DAYS(year,months) yeardays (HAS_NOLEAPDAY (year), months)
#define YEAR_ALL_DAYS(year)    YEAR_DAYS (year, 12)

/* Return the number of days for the specified months since January in
   a year if its value is from 0 to 12, otherwise, -1.  */

int yeardays(bool has_noleapday, int months);

/* Calculate the number of leap days between the specified years. If TO_YEAR
   is greater than, equal to, or less than FROM_YEAR, return the positive
   zero, or negateive value, but if the difference of tow years is overflow,
   return INT_MAX or INT_MIN.  */

int leapdays (int from_year, int to_year);

/* Return the week day for the specified day in YEAR.  */

int weekday (int year, int yday);

/* Seconds in a day  */

#define SECONDS_IN_DAY 86400

/* Seconds at a time  */

#define SECONDS_AT(hour,min,sec) ((hour) * 3600 + (min) * 60 + (sec))

/* The year which is set as a zero into the dtm struct  */

#ifndef TM_YEAR_BASE
# define TM_YEAR_BASE 1900
#endif

/* Parameters of date  */

struct dtm
{
  int tm_mday;
  int tm_mon;
  int tm_year;
  int tm_yday;
  int tm_wday;
};

/* Bring the tm_mday member in *TM into the range of 0 to the last day in
   a month and adjust other members by changed days. But the tm_wday member
   is set if negative. If adjustment is performed, return true and overwrite
   *TM by those values, otherwise, return false and never change.  */

bool adjustday (struct dtm *tm);

/* Decrease or increase PARAM2 toward zero by the multiple of BASE as many as
   possible and increase or decrease PARAM1 by its multiple of UNIT if PARAM2
   is positive or negative. Return true if not overflow or divided by zero,
   otherwise, false.  */

bool adjusttm (int *param1, int unit, int *param2, int base);

/* Bring LOWPARAM into the range of 0 to BASE - 1 and increase or decrease
   HIGHPARAM by its carried value if LOWPARAM is positive or negative. Return
   true if not overflow or divided by zero, otherwise, false.  */

bool carrytm (int *highparam, int *lowparam, int base);
