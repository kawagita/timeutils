/* Get the current time by the system clock in NTFS
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

#include "config.h"

#ifndef USE_TM_CYGWIN
# include <windows.h>
#endif
#include <stdbool.h>
#include <stdint.h>

#ifdef USE_TM_CYGWIN
# include <time.h>
#endif

#include "wintm.h"

/* Get the current time in NTFS into *FT. Return true if conversion is
   performed, otherwise, false.  */

bool
gettime (FT *ft)
{
#ifdef USE_TM_CYGWIN
  if (clock_gettime (CLOCK_REALTIME, ft) != 0)
    return false;

#else  /* USE_TM_MSVCRT || USE_TM_WRAPPER */
  GetSystemTimeAsFileTime (ft);
#endif

  return true;
}

#ifdef TEST
# include <stdio.h>
# include <unistd.h>

# include "exit.h"
# include "printtm.h"

static void
usage (int status)
{
  fputs ("Usage: gettime [OPTION]...\n", stdout);
# ifdef USE_TM_CYGWIN
  fputs ("\
Display current time in seconds since 1970-01-01 00:00 UTC.\n\
\n\
Options:\n\
", stdout);
# else
  fputs ("\
Display current time in 100 nanoseconds since 1601-01-01 00:00 UTC.\n\
\n\
Options:\n\
", stdout);
# endif
# ifndef USE_TM_CYGWIN
  fputs ("\
  -E   output time in seconds since 1970-01-01 00:00 UTC\n\
", stdout);
# else
  fputs ("\
  -F   output time in 100 nanoseconds since 1601-01-01 00:00 UTC\n\
", stdout);
# endif
  fputs ("\
  -n   don't output the trailing newline\n\
  -N   don't output nanoseconds\n\
", stdout);
  exit (status);
}

int
main (int argc, char **argv)
{
  struct tmout_fmt ft_fmt = { false };
  struct tmout_ptrs ft_ptrs = { NULL };
  FT ft;
  intmax_t ft_elapse = 0;
  int ft_frac = 0;
  int c;
  bool success;
  bool seconds_set = false;
  bool nsec_set = true;

# ifdef USE_TM_CYGWIN
  seconds_set = true;
  ft_elapse = -1;
# endif
  ft_ptrs.tm_elapse = &ft_elapse;

  while ((c = getopt (argc, argv, ":EFnN")) != -1)
    {
      switch (c)
        {
# ifndef USE_TM_CYGWIN
        case 'E':
          seconds_set = true;
          break;
# else
        case 'F':
          seconds_set = false;
          break;
# endif
        case 'n':
          ft_fmt.no_newline = true;
          break;
        case 'N':
          nsec_set = false;
          break;
        default:
          usage (EXIT_FAILURE);
        }
    }

  if (argc > optind)
    usage (EXIT_FAILURE);

  success = gettime (&ft);

  if (success)
    {
      if (seconds_set)  /* Seconds since Unix epoch */
        {
          success = ft2secns (&ft, &ft_elapse, &ft_frac);

          /* If a time is overflow for time_t of 32 bits, output "-1". */
          if (success && nsec_set)
            ft_ptrs.tm_frac = &ft_frac;
        }
      else  /* 100 nanoseconds since 1601-01-01 00:00 UTC */
        {
          ft_elapse = toftval (&ft);

          if (!nsec_set)
            ft_elapse = ft_elapse / FT_FRAC_PRECISION * FT_FRAC_PRECISION;
        }
    }

  printtm (&ft_fmt, &ft_ptrs);

  return success ? EXIT_SUCCESS : EXIT_FAILURE;
}
#endif
