/* localtime.h -- Conversion from Unix seconds to local time in NTFS

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

/* Convert the specified seconds since Unix epoch to local time and set
   parameters of time into *TM. Return the pointer to it if conversion is
   performed, otherwise, NULL. But never change the tm_gmtoff member in
   *TM. */

TM *localtimew (const intmax_t *seconds, TM *tm);
