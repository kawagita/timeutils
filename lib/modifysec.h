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
#define NSEC_PERMUTE       8

/* Return true if each modification flag is set  */

#define IS_SECONDS_ROUND_UP(modflag)   (modflag & SECONDS_ROUND_UP)
#define IS_SECONDS_ROUND_DOWN(modflag) (modflag & SECONDS_ROUND_DOWN)
#define IS_NSEC_RANDOM(modflag)        (modflag & NSEC_RANDOM)
#define IS_NSEC_PERMUTE(modflag)       (modflag & NSEC_PERMUTE)

/* Return true if seconds is rounded up or down to the largest or smallest
   that is more than or not less than its value by the specified flag  */

#define IS_SECONDS_ROUNDING(modflag) \
          (modflag & (SECONDS_ROUND_DOWN | SECONDS_ROUND_UP))

/* Return true if the random value of 100 nanoseconds less than a second
   is used by the specified flag  */

#define IS_NSEC_RANDOMIZING(modflag) (modflag & (NSEC_RANDOM | NSEC_PERMUTE))

/* Modify the specified value of seconds since 1970-01-01 00:00 UTC and 100
   nanoseconds less than a second, according to MODFLAG. Set its two values
   back into *SECONDS and *NSEC and return true if *NSEC is not less than 0
   and modification is performed, otherwise, return false.  */

bool modifysec (intmax_t *seconds, int *nsec, int modflag);
