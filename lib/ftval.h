/* ftval.h -- The value of 100 nanoseconds since 1601-01-01 00:00 UTC

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

/* The value of a second in FILETIME  */

#define FILETIME_SECOND_VALUE 10000000

/* Maximum and minimum second of a value in FILETIME  */

#define MAX_SECOND_IN_FILETIME  922337203685
#define MIN_SECOND_IN_FILETIME -922337203685

/* The value to 1970-01-01 00:00 UTC in FILETIME  */

#define FILETIME_UNIXEPOCH_VALUE 116444736000000000
