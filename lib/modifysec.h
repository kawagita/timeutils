/* modifysec.h -- Modification of Unix seconds

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

/* Generate a new sequence of pseudo-random values for the specified seed
   number. If SEED is less than 0, use current time instead.  */

void srandsec (int seed);

/* The modification flag of seconds or 100 nanoseconds less than a second  */

#define SECONDS_ROUND_UP   1
#define SECONDS_ROUND_DOWN 2
#define NSEC_RANDOM        4
#define NSEC_ARRANGE       8

/* Return true if seconds is rounded down or up by the specified flag  */

#define SECONDS_ROUNDING(flag) (flag & (SECONDS_ROUND_DOWN | SECONDS_ROUND_UP))

/* Modify the specified value of seconds since 1970-01-01 00:00 UTC and 100
   nanoseconds less than a second, according to MODFLAG. Set its two values
   back into *SECONDS and *NSEC and return true if *NSEC is not less than 0
   and modification is performed, otherwise, return false.  */

bool modifysec (intmax_t *seconds, int *nsec, int modflag);
