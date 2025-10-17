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

#include <windows.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef USE_TM_CYGWIN
# include <time.h>
#endif

#include "wintm.h"

/* Get the current time into the specified file time and return true
   if conversion is performed, otherwise, false.  */

bool
gettime (FILETIME *ft)
{
#ifdef USE_TM_CYGWIN
  struct timespec ts;

  if (clock_gettime (CLOCK_REALTIME, &ts) != 0
      || ! SECONDS_NSEC_TO_FILETIME (ts.tv_sec, ts.tv_nsec / 100, ft))
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
  fputs ("\
Display the current time in seconds since 1970-01-01 00:00 UTC.\n\
\n\
Options:\n\
  -n   don't output the trailing newline\n\
  -N   output the fractional part of seconds\n\
", stdout);
  exit (status);
}

int
main (int argc, char **argv)
{
  struct tmout_fmt sec_fmt = { false };
  struct tmout_ptrs sec_ptrs = { NULL };
  FILETIME ft;
  intmax_t seconds = -1;
  int nsec = -1;
  int c;
  int status = EXIT_FAILURE;

  sec_ptrs.tm_elapse = &seconds;

  while ((c = getopt (argc, argv, ":nN")) != -1)
    {
      switch (c)
        {
        case 'n':
          sec_fmt.no_newline = true;
          break;
        case 'N':
          sec_ptrs.tm_frac = &nsec;
          break;
        default:
          usage (EXIT_FAILURE);
        }
    }

  if (argc > optind)
    usage (EXIT_FAILURE);
  else if (gettime (&ft) && FILETIME_TO_SECONDS_NSEC (&ft, &seconds, &nsec))
    status = EXIT_SUCCESS;

  printtm (&sec_fmt, &sec_ptrs);

  return status;
}
#endif
