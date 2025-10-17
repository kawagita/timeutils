/* printtm.h -- Outputting parameters of time to standard output

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

/* The format of time output to standard output  */

struct tmout_fmt
{
  bool weekday_name;
  bool week_numbering;
  bool relative;
  bool iso8601;
  bool japanese;
  bool no_newline;
};

/* Pointers to parameters of time output to standard output  */

struct tmout_ptrs
{
  intmax_t *tm_elapse;
  const char *elapse_delim;
  int *tm_year;
  int *tm_mon;
  int *tm_mday;
  int *tm_wday;
  int *tm_yday;
  int *tm_hour;
  int *tm_min;
  int *tm_sec;
  int *tm_frac;
  long int *tm_gmtoff;
  int *tm_isdst;
};

/* Output each parameter of time included in *TM_PTRS to standard output
   if its pointer is not NULL, according to the format of members in
   *TM_FMT. Return the number of output parameters.  */

int printtm (const struct tmout_fmt *tm_fmt, const struct tmout_ptrs *tm_ptrs);
