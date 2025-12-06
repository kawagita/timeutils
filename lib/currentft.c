/* Get the current time from the system clock in NTFS
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

#ifdef USE_TM_GLIBC
# include <time.h>
#else
# include <windows.h>
#endif
#include <stdbool.h>
#include <stdint.h>

#include "ft.h"

/* Get the current time on system clock as file time in NTFS. Set its value
   into *FT and return true if successfull, otherwise, return false.  */

bool
currentft (FT *ft)
{
#ifdef USE_TM_GLIBC
  if (clock_gettime (CLOCK_REALTIME, ft) != 0)
    return false;

#else
  GetSystemTimeAsFileTime (ft);
#endif

  return true;
}

#ifdef TEST
# include <unistd.h>

# include "cmdtmio.h"
# include "exit.h"

static void
usage (int status)
{
  printusage ("currentft", "\n\
Display current time " IN_DEFAULT_TIME ".\n\
\n\
Options:\n"
# ifdef USE_TM_GLIBC
"  -v   output time " IN_FILETIME
# else
"  -s   output time " IN_UNIX_SECONDS
# endif
, true, false, 0);
  exit (status);
}

int
main (int argc, char **argv)
{
  FT ft;
  intmax_t ft_elapse = 0;
  int ft_frac_val = -1;
  int c;
  bool success;
  bool seconds_output = false;

# ifdef USE_TM_GLIBC
  seconds_output = true;
  ft_elapse = -1;
# endif

  while ((c = getopt (argc, argv, ":sv")) != -1)
    {
      switch (c)
        {
# ifdef USE_TM_GLIBC
        case 'v':
          seconds_output = false;
          ft_elapse = 0;
          break;
# else
        case 's':
          seconds_output = true;
          ft_elapse = -1;
          break;
# endif
        default:
          usage (EXIT_FAILURE);
        }
    }

  if (argc > optind)
    usage (EXIT_FAILURE);

  success = currentft (&ft);

  if (success)
    {
      if (seconds_output)  /* Seconds since 1970-01-01 00:00 UTC */
        {
          int frac_val;

          success = ft2sec (&ft, &ft_elapse, &frac_val);

          if (success)
            ft_frac_val = frac_val;
        }
      else  /* 100 nanoseconds since 1601-01-01 00:00 UTC */
        success = ft2val (&ft, 0, &ft_elapse);
    }

  printelapse (false, ft_elapse, ft_frac_val);

  return success ? EXIT_SUCCESS : EXIT_FAILURE;
}
#endif
