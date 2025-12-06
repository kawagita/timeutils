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

/* Function to get or set nanoseconds less than a seconds in file time  */

#if defined _WIN32 || defined __CYGWIN__
# define GET_FT_NSEC(ft)    ((ft)->tv_nsec / 100)
# define SET_FT_NSEC(ft,ns) (ft)->tv_nsec = (ns) * 100
#else
# define GET_FT_NSEC(ft)    (ft)->tv_nsec
# define SET_FT_NSEC(ft,ns) (ft)->tv_nsec = ns
#endif

/* The value used by the calculation or outputting of nanoseconds less than
   a seconds in file time  */

#if defined _WIN32 || defined __CYGWIN__
# define FT_NSEC_PRECISION 10000000
# define FT_NSEC_DIGITS    7
# define FT_NSEC_FORMAT   ".%07d"
# define FT_NSEC_NOTATION ".nnnnnnn"
#else
# define FT_NSEC_PRECISION 1000000000
# define FT_NSEC_DIGITS    9
# define FT_NSEC_FORMAT   ".%09d"
# define FT_NSEC_NOTATION ".nnnnnnnnn"
#endif

/* Return true if the specified seconds and nanoseconds less than a second
   is outside the range of time_t value in file time.  */

bool secoverflow (intmax_t seconds, int nsec);

/* Modification flags of seconds or nanoseconds less than a second  */

#define FT_SECONDS_ROUND_UP   1
#define FT_SECONDS_ROUND_DOWN 2
#define FT_NSEC_RANDOM        4
#define FT_NSEC_PERMUTE       8

/* Return true if file time is set by each modification flag  */

#define IS_FT_SECONDS_ROUND_UP(modflag)   (modflag & FT_SECONDS_ROUND_UP)
#define IS_FT_SECONDS_ROUND_DOWN(modflag) (modflag & FT_SECONDS_ROUND_DOWN)
#define IS_FT_NSEC_RANDOM(modflag)        (modflag & FT_NSEC_RANDOM)
#define IS_FT_NSEC_PERMUTE(modflag)       (modflag & FT_NSEC_PERMUTE)

/* Return true if seconds is rounded up or down in file time, according
   to the specified modification flag  */

#define IS_FT_SECONDS_ROUNDING(modflag) \
          (modflag & (FT_SECONDS_ROUND_DOWN | FT_SECONDS_ROUND_UP))

/* Return true if the random value is set into nanoseconds less than
   a second in file time, according to the specified modification flag  */

#define IS_FT_NSEC_RANDOMIZING(modflag) \
          (modflag & (FT_NSEC_RANDOM | FT_NSEC_PERMUTE))

/* Modify the specified value of seconds since 1970-01-01 00:00 UTC and
   nanoseconds less than a second, according to MODFLAG. Set its two values
   back into *SECONDS and *NSEC and return true if *NSEC is not less than 0
   and modification is performed, otherwise, return false.  */

bool modifysec (intmax_t *seconds, int *nsec, int modflag);

/* Generate a new sequence of pseudo-random values for the specified seed
   number in the modification of nanoseconds less than a second. If SEED
   is less than 0, use current time instead.  */

void srandsec (int seed);
