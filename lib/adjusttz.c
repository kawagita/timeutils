/* Adjust parameters of time by the information of time zone
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

#include <windows.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>

#include "adjusttm.h"
#include "adjusttz.h"
#include "imaxoverflow.h"
#include "intoverflow.h"

/* Days in a month  */
static const int mdays[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

/*
   Return seconds in which the transition occurs at the specified system time,
   elapsed in a year starting the specified day of week.  */
static int
transition_seconds (SYSTEMTIME *st, int y1st_wday, bool has_noleapday)
{
  int trans_yday = yeardays (has_noleapday, st->wMonth - 1);
  int trans_mday = WEEKDAY_FROM (0, st->wDayOfWeek - (trans_yday + y1st_wday))
                   + (st->wDay - 1) * 7 + 1;

  if (trans_mday > ((has_noleapday || st->wMonth ^ 2)
                    ? mdays[st->wMonth - 1] : mdays[st->wMonth - 1] + 1))
    trans_mday -= 7;

  return (trans_yday + trans_mday - 1) * SECONDS_IN_DAY
         + SECONDS_AT (st->wHour, st->wMinute, st->wSecond);
}

/* Seconds elapsed since 00:00, Jan 1st in a year for SYSTEMTIME  */
#define SYSTEMTIME_SECONDS(st,has_nolday) \
  ((yeardays (has_nolday, (st).wMonth - 1) + (st).wDay - 1) * SECONDS_IN_DAY \
   + SECONDS_AT ((st).wHour, (st).wMinute, (st).wSecond))

/* Return true as DST is in effect for the positive ISDST  */
#define DST_EFFECT(isdst) ((isdst) > 0 ? true : false)

/* Set the tm_isdst member to a positive value or zero if DST is in effect
   for seconds of the tm_ysec member in the tm_year, inculded in *TM. If DST
   is in effect or not for seconds that is either skipped over or repeated
   when a transition to or from DST occurs, specify a positive value or zero
   to TRANS_ISDST, otherwise, attempt to determine whether the specified
   seconds is included in the term of DST. Adjust the tm_min and tm_gmtoff
   member by the information of time zone. If adjustment is performed,
   return true and overwrite *TM by those values, otherwise, return false
   and never change.  */

bool
adjusttz (struct lctm *tm, int trans_isdst)
{
  TIME_ZONE_INFORMATION tzinfo;
  int year;
  int min = tm->tm_min;
  int isdst = tm->tm_isdst;
  intmax_t offset = 0;
  bool dst_effect = false;

  if (INT_ADD_WRAPV (tm->tm_year, TM_YEAR_BASE, &year))
    return false;
#if _WIN32_WINNT >= 0x0601
  else if (! GetTimeZoneInformationForYear ((year >= 0
               ? (year > USHRT_MAX ? USHRT_MAX : year) : 0), NULL, &tzinfo))
    return false;
#else
  GetTimeZoneInformation (&tzinfo);
#endif

  if (tzinfo.DaylightDate.wMonth > 0)
    {
      bool has_noleapday = HAS_NOLEAPDAY (year);
      int st_trans, dst_trans;
      intmax_t adj_min = 0;

      if (tzinfo.DaylightDate.wYear > 0)  /* Only occur one time */
        {
          if (year != tzinfo.DaylightDate.wYear)
            {
              tm->tm_isdst = -1;
              tm->tm_gmtoff = 0;

              return false;
            }

          st_trans = SYSTEMTIME_SECONDS (tzinfo.StandardDate, has_noleapday);
          dst_trans = SYSTEMTIME_SECONDS (tzinfo.DaylightDate, has_noleapday);
        }
      else  /* occurs yearly */
        {
          int y1st_wday = weekday (year, 0);

          st_trans = transition_seconds (
                       &(tzinfo.StandardDate), y1st_wday, has_noleapday);
          dst_trans = transition_seconds (
                        &(tzinfo.DaylightDate), y1st_wday, has_noleapday);
        }

      if (tm->tm_ysec >= st_trans - 3600 && tm->tm_ysec < st_trans)
        /* Time in seconds repeated when transition from DST occurs */
        {
          if (isdst < 0)
            {
              if (trans_isdst < 0)
                dst_effect = false;
              else
                {
                  dst_effect = DST_EFFECT (trans_isdst);
                  isdst = dst_effect ? 1 : 0;
                }
            }
          else
            dst_effect = DST_EFFECT (isdst);
        }
      else if (tm->tm_ysec >= dst_trans && tm->tm_ysec < dst_trans + 3600)
        /* Tiem in seconds skipped over when transition to DST occurs */
        {
          if (isdst < 0)
            {
              if (trans_isdst < 0)
                dst_effect = true;
              else
                {
                  dst_effect = DST_EFFECT (trans_isdst);
                  isdst = dst_effect ? 0 : 1;
                }
            }
          else
            dst_effect = ! DST_EFFECT (isdst);
        }
      else if (st_trans < dst_trans
               ? (tm->tm_ysec < st_trans || tm->tm_ysec >= dst_trans)
               : (tm->tm_ysec >= dst_trans && tm->tm_ysec < st_trans))
        /* Time in seconds included in the term of Daylight Saving Time */
        dst_effect = true;
      else
        /* Time in seconds included in the term of Standard time */
        dst_effect = false;

      /* Alter fields appropriately but it's unspecified by POSIX.1-2024. */
      if (dst_effect)
        {
          /* If zero is given for tm_isdst when DST is in effect, given
             back the positive value according to POSIX.1-2024 and subtract
             the DST offset from adjusted parameters of time. */
          if ((isdst == 0
               && IMAX_SUBTRACT_WRAPV (
                    tzinfo.StandardBias, tzinfo.DaylightBias, &adj_min))
              || IMAX_SUBTRACT_WRAPV (offset, tzinfo.DaylightBias, &offset))
            return false;
        }
      else if ((isdst > 0
                && IMAX_SUBTRACT_WRAPV (
                     tzinfo.DaylightBias, tzinfo.StandardBias, &adj_min))
               || IMAX_SUBTRACT_WRAPV (offset, tzinfo.StandardBias, &offset))
        return false;

      /* Add minutes adjusted by the ST and DT Bias to the tm_mim member. */
      if (adj_min < INT_MIN || adj_min > INT_MAX
          || INT_ADD_WRAPV (min, adj_min, &min))
        return false;
    }
  else if (isdst > 0 && trans_isdst > 0 && INT_SUBTRACT_WRAPV (min, 60, &min))
    return false;

  /* Set the tm_gmtoff member to - (tzinfo.Bias + (ST or DT Bias)) * 60. */
  if (IMAX_SUBTRACT_WRAPV (offset, tzinfo.Bias, &offset)
      || IMAX_MULTIPLY_WRAPV (offset, 60, &offset)
      || offset < LONG_MIN || offset > LONG_MAX)
    return false;

  tm->tm_min = min;
  tm->tm_isdst = dst_effect ? 1 : 0;
  tm->tm_gmtoff = offset;

  return true;
}
