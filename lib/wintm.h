/* wintm.h -- Parameters of time in NTFS

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

/* The start year in file time  */

#define FILETIME_YEAR_MIN 1601

/* The precision and number of digits for a fractional second in file time  */

#define FILETIME_FRAC_PRECISION 10000000
#define FILETIME_FRAC_DIGITS    7

/* Seconds to Unix epoch in file time  */

#define FILETIME_UNIXEPOCH_SECONDS 11644473600LL

/* Convert the specified file time to seconds and 100 nanoseconds since Unix
   epoch. Set its value into *SECONDS but nanoseconds into *NSEC unless not
   not NULL. Return true if conversion is performed, otherwise, false.  */

bool ft2secns (const FILETIME *ft, intmax_t *seconds, int *nsec);

#define FILETIME_TO_SECONDS_NSEC(ft,seconds,nsec) ft2secns (ft, seconds, nsec)
#define FILETIME_TO_SECONDS(ft,seconds)           ft2secns (ft, seconds, NULL)

/* Convert the specified seconds and 100 nanoseconds since Unix epoch to
   file time and set its value into *FT. If NSEC is less than 0, never change
   it. Return true if conversion is performed, otherwise, false.  */

bool secns2ft (const intmax_t seconds, const int nsec, FILETIME *ft);

#define SECONDS_NSEC_TO_FILETIME(seconds,ns,ft) secns2ft (seconds, ns, ft)
#define SECONDS_TO_FILETIME(seconds,ft)         secns2ft (seconds, 0, ft)

/* The year which is set as a zero into the tm struct  */

#ifndef TM_YEAR_BASE
# define TM_YEAR_BASE 1900
#endif

#ifdef USE_TM_CYGWIN
# define TM struct tm
#else
# define TM struct wintm

/* Parameters of time to use in the environment of Windows  */

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
