/* wintm.h -- Parameters of time used for POSIX function in Windows

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

/* The year which is set as a zero into the struct TM  */

#ifndef TM_YEAR_BASE
# define TM_YEAR_BASE 1900
#endif

/* The structure of a calendar date and time   */

#ifdef USE_TM_CYGWIN
typedef struct tm TM;
#else
typedef struct wintm TM;

/* Parameters of time to use for POSIX function in the environment
   of Windows, instead of struct tm  */

struct wintm
{
  int tm_sec;
  int tm_min;
  int tm_hour;
  int tm_mday;
  int tm_mon;
  int tm_year;
  int tm_wday;
  int tm_yday;
  int tm_isdst;
  long int tm_gmtoff;
};
#endif
