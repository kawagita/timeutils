/* mktime.h -- Conversion from parameters of time to Unix seconds

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

/* Convert the specified parameters of *TM into seconds since Unix epoch,
   and adjust each parameter to the range of correct values. If successful,
   return the number of converted seconds and overwrite *TM by adjusted
   values, otherwise, return -1 and never overwrite those.  */

intmax_t mktimew (TM *tm);
