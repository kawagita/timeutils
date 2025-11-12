/* Convert the argument into parameters of time
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

#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>

#include "cmdtmio.h"
#include "imaxoverflow.h"
#include "intoverflow.h"

static int
sscanelapse (const char *argv, intmax_t *elapse, int *sign, char **endptr)
{
  struct numimax_prop elapse_prop = { 1, INTMAX_MIN, INTMAX_MAX, 0 };

  while (isspace (*argv))
    argv++;

  if (*argv == '-')
    {
      elapse_prop.sign = -1;
      argv++;
    }

  *sign = elapse_prop.sign;

  return sscannumimaxp (argv, &elapse_prop, elapse, NULL, endptr);
}

/* Parse the leading part of the specified argument as parameters of time
   included in *TM_PTRS and set those values into the member, storing
   the pointer to a following character into *ENDPTR. Return the number
   of set values, otherwise, -1 if a value is outside the range of its
   parameter.  */

int
sscantm (const char *argv, struct tm_ptrs *tm_ptrs, char **endptr)
{
  bool elapse_leading = false;
  bool times_leading = false;
  int *frac_val = NULL;
  int frac_sign = 1;
  int set_num = 0;

  /* Input seconds or nanoseconds elapsed since a time. */
  if (tm_ptrs->elapse)
    {
      set_num = sscanelapse (argv, tm_ptrs->elapse, &frac_sign, endptr);
      if (set_num <= 0)
        return set_num;

      elapse_leading = true;
      frac_val = tm_ptrs->frac_val;
      argv = *endptr;
    }

  /* Input an abbreviation for the week day. */
  if (tm_ptrs->weekday)
    {
      struct word_table weekday_table[] =
        {
          { "SUNDAY", 0 }, { "MONDAY", 1 }, { "TUESDAY", 2 },
          { "WEDNESDAY", 3 }, { "THURSDAY", 4 }, { "FRIDAY", 5 },
          { "SATURDAY", 6 }, { NULL }
        };
      int weekday;
      if (sscanword (argv, weekday_table, 3, &weekday, endptr))
        {
          argv = *endptr;

          if (tm_ptrs->weekday_ordinal)
            {
              intmax_t ordinal = 0;
              if (*argv == ',')
                {
                  int ordinal_num = sscannumimax (++argv, &ordinal, endptr);
                  if (ordinal_num < 0)
                    return ordinal_num;
                  else if (ordinal_num == 0)
                    return set_num;
                 }

              *tm_ptrs->weekday_ordinal = ordinal;
              argv = *endptr;
            }

          *tm_ptrs->weekday = weekday;
          elapse_leading = false;
          set_num++;
        }
    }

  /* Input the date and time. */
  if (tm_ptrs->dates)
    {
      const struct numint_prop tmint_prop = { 1, 0, INT_MAX, 0 };
      const struct numint_prop date_props[] =
        {
          { 0, -1, INT_MAX, 0 }, tmint_prop, tmint_prop
        };
      int i;
      for (i = 0; i < 3; i++)
        {
          bool negative = false;

          if (i > 0)
            {
              if (*argv == '+')
                negative = true;
              else if (*argv != '-')
                return set_num;

              argv++;
            }

          int date_num = sscannumintp (argv, date_props + i,
                                       tm_ptrs->dates[i], NULL, endptr);
          if (date_num < 0)
            return date_num;
          else if (date_num == 0)
            return set_num;
          else if (negative)
            *tm_ptrs->dates[i] = - *tm_ptrs->dates[i];

          set_num++;
          argv = *endptr;
        }

      /* Intput the hour, minutes, and seconds. */
      if (tm_ptrs->times)
        {
          if (*argv != 'T')
            return set_num;

          for (i = 0; i < 3; i++)
            {
              if (i > 0 && *argv != ':')
                return set_num;

              int time_num = sscannumintp (++argv, &tmint_prop,
                                           tm_ptrs->times[i], NULL, endptr);
              if (time_num < 0)
                return time_num;
              else if (time_num == 0)
                return set_num;

              set_num++;
              argv = *endptr;
            }

          frac_val = tm_ptrs->frac_val;
          times_leading = true;
        }

      elapse_leading = false;
    }

  /* Input the value of fractional part in seconds. */
  if (frac_val)
    {
      if (*argv != '.' && *argv != ',')
        return set_num;

      /* If a fractional part is leading by the negative value of seconds,
         its value is returned with the addition to 1.0 and must decrement
         seconds. */
      struct numint_prop frac_prop =
        { frac_sign, 0, TM_FRAC_MAX, TM_FRAC_DIGITS };
      int elapse_decr = 0;
      int *intdecr = elapse_leading ? &elapse_decr : NULL;
      int frac_num = sscannumintp (argv + 1, &frac_prop,
                                   frac_val, intdecr, endptr);
      if (frac_num < 0)
        return frac_num;
      else if (frac_num == 0)
        return set_num;
      else if (elapse_decr
               && IMAX_SUBTRACT_WRAPV (*tm_ptrs->elapse, elapse_decr,
                                       tm_ptrs->elapse))
        return -1;

      set_num++;
      argv = *endptr;
    }

  /* Intput the offset of time zone. */
  if (tm_ptrs->tz_offset && times_leading)
    {
      struct numint_prop tz_prop = { 0, -9999, 9999, 0 };
      int tz_hhmm = 0;
      int tz_num = sscannumintp (argv, &tz_prop, &tz_hhmm, NULL, endptr);
      if (tz_num <= 0)
        return tz_num;
      else if (tz_num == 0)
        return set_num;

      *tm_ptrs->tz_offset = ((tz_hhmm / 100) * 60 + tz_hhmm % 100) * 60;
      set_num++;
    }

  /* Intput "DST" or "ST" if DST is in effect or not, otherwise, "AUTO". */
  if (tm_ptrs->tz_isdst)
    {
      struct word_table isdst_table[] =
        { { "DST", 1 }, { "ST", 0 }, { "AUTO", -1 }, { NULL } };
      int tz_isdst;
      if (sscanword (argv, isdst_table, 0, &tz_isdst, endptr))
        {
          *tm_ptrs->tz_isdst = tz_isdst;
          set_num++;
        }
    }

  return set_num;
}

/* Parse the specified arguments as relative parameters of time included
   in *TM_PTRS and set those values into the member, storing the pointer
   to a following character into *ENDPTR. Return the number of set values,
   otherwise, -1 if a value is outside the range of its parameter.  */

int
sscanreltm (int argc, const char **argv,
            struct tm_ptrs *tm_ptrs, char **endptr)
{
  int set_num = 0;

  /* Input the relative date and time. */
  if (tm_ptrs->dates)
    {
      const struct numint_prop date_props[] =
        {
          { 0, INT_MIN + TM_YEAR_BASE, INT_MAX, 0 },
          { 0, INT_MIN + 1, INT_MAX, 0 },
          { 0, INT_MIN, INT_MAX, 0 }
        };
      int i;
      for (i = 0; i < 3; i++)
        {
          if (--argc < 0)
            return set_num;

          int date_num;
          if (tm_ptrs->rel_times)
            date_num = sscannumint (*argv, tm_ptrs->dates[i], endptr);
          else
            date_num = sscannumintp (*argv, date_props + i,
                                     tm_ptrs->dates[i], NULL, endptr);
          if (date_num < 0)
            return date_num;
          else if (date_num == 0 || **endptr != '\0')
            return set_num;

          set_num++;
          argv++;
        }

      /* Intput the relative hour, minutes, and seconds. */
      if (tm_ptrs->times || tm_ptrs->rel_times)
        {
          for (i = 0; i < 3; i++)
            {
              if (--argc < 0)
                return set_num;

              int time_num;
              if (tm_ptrs->times)
                time_num = sscannumint (*argv, tm_ptrs->times[i], endptr);
              else
                time_num = sscannumimax (*argv, tm_ptrs->rel_times[i], endptr);
              if (time_num <= 0)
                return time_num;
              else if (time_num == 0 || **endptr != '\0')
                return set_num;

              set_num++;
              argv++;
            }

          /* Input the relative value of fractional part in seconds. */
          if (tm_ptrs->frac_val && --argc >= 0)
            {
              const struct numint_prop frac_prop =
                { 0, TM_FRAC_MIN, TM_FRAC_MAX, TM_FRAC_DIGITS };
              int frac_num = sscannumintp (*argv, &frac_prop,
                                           tm_ptrs->frac_val, NULL, endptr);
              if (frac_num <= 0)
                return frac_num;
              else if (frac_num == 0 || **endptr != '\0')
                return set_num;

              set_num++;
            }
        }
    }

  return set_num;
}
