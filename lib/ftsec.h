/* ftsec.h -- Seconds and nanoseconds less than a second in file time

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

/* Maximum and minimum value of seconds in file time  */

#define FT_SECONDS_MAX  922337203685
#define FT_SECONDS_MIN -922337203685

/* The precision and number of digits for nanoseconds less than a seconds
   in file time  */

#define FT_NSEC_PRECISION 10000000
#define FT_NSEC_DIGITS    7

/* The value to 1970-01-01 00:00 UTC in file time  */

#define FT_UNIXEPOCH_VALUE 116444736000000000

/* Return true if the specified seconds and 100 nanoseconds less than
   a secondis is outside the range of time_t value in file time.  */

bool secoverflow (intmax_t seconds, int nsec);

/* Modification flags of seconds or 100 nanoseconds less than a second  */

#define FT_SECONDS_ROUND_UP   1
#define FT_SECONDS_ROUND_DOWN 2
#define FT_NSEC_RANDOM        4
#define FT_NSEC_PERMUTE       8

/* Return true if file time is set by each modification flag  */

#define IS_FT_SECONDS_ROUND_UP(modflag)   (modflag & FT_SECONDS_ROUND_UP)
#define IS_FT_SECONDS_ROUND_DOWN(modflag) (modflag & FT_SECONDS_ROUND_DOWN)
#define IS_FT_NSEC_RANDOM(modflag)        (modflag & FT_NSEC_RANDOM)
#define IS_FT_NSEC_PERMUTE(modflag)       (modflag & FT_NSEC_PERMUTE)

/* Return true if seconds is rounded up or down to the largest or smallest
   that is more than or not less than its value, according to the specified
   modification flag  */

#define IS_FT_SECONDS_ROUNDING(modflag) \
          (modflag & (FT_SECONDS_ROUND_DOWN | FT_SECONDS_ROUND_UP))

/* Return true if the random value is set into 100 nanoseconds less than
   a second, according to the specified modification flag  */

#define IS_FT_NSEC_RANDOMIZING(modflag) \
          (modflag & (FT_NSEC_RANDOM | FT_NSEC_PERMUTE))

/* Modify the specified value of seconds since 1970-01-01 00:00 UTC and 100
   nanoseconds less than a second, according to MODFLAG. Set its two values
   back into *SECONDS and *NSEC and return true if *NSEC is not less than 0
   and modification is performed, otherwise, return false.  */

bool modifysec (intmax_t *seconds, int *nsec, int modflag);

/* Generate a new sequence of pseudo-random values for the specified seed
   number. If SEED is less than 0, use current time instead.  */

void srandsec (int seed);
