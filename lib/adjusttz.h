/* adjusttz.h -- Adjustment of time by the information of time zone

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

/* Parameters of local time adjusted by time zone  */

struct lctm
{
  int tm_year;
  int tm_ysec;
  int tm_min;
  int tm_isdst;
  long tm_gmtoff;
};

/* Set the tm_isdst member to a positive value or zero if DST is in effect
   for seconds of the tm_ysec member in the tm_year, inculded in *TM. If DST
   is in effect or not for seconds that is either skipped over or repeated
   when a transition to or from DST occurs, specify a positive value or zero
   to TRANS_ISDST, otherwise, attempt to determine whether the specified
   seconds is included in the term of DST. Adjust the tm_min and tm_gmtoff
   member by the information of time zone. If adjustment is performed,
   return true and overwrite *TM by those values, otherwise, return false
   and never change.  */

bool adjusttz (struct lctm *tm, int trans_isdst);
