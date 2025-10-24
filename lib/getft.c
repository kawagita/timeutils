/* Get the last access, last write, and creation time of a file
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
# include <sys/stat.h>
# include <time.h>
#endif

#include "wintm.h"

/* Get file times for the specified struct file into FT and set the flag
   of a directory into the isdir member in *FT_FILE. If the no_dereference
   member is true, get the time of symbolic link on Cygwin but not a file
   referenced by it. Return true if successfull, otherwise, false.  */

bool
getft (FT ft[FT_SIZE], struct file *ft_file)
{
#ifdef USE_TM_CYGWIN
  struct stat st;
  const char *fname = ft_file->name;

  if ((ft_file->no_dereference ? lstat (fname, &st) : stat (fname, &st)) == 0)
    {
      ft[FT_ATIME] = st.st_atim;
      ft[FT_MTIME] = st.st_mtim;

      ft_file->isdir = S_ISDIR (st.st_mode);

      return true;
    }
#else
  WIN32_FILE_ATTRIBUTE_DATA finfo;
  LPCWSTR fname = ft_file->name;

  if (GetFileAttributesEx (fname, GetFileExInfoStandard, &finfo))
    {
      ft[FT_ATIME] = finfo.ftLastAccessTime;
      ft[FT_MTIME] = finfo.ftLastWriteTime;
      ft[FT_CTIME] = finfo.ftCreationTime;

      ft_file->isdir = finfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY != 0;

      return true;
    }
#endif

  return false;
}

#ifdef TEST
# include <limits.h>
# include <stdio.h>
# include <unistd.h>

# include "error.h"
# include "exit.h"
# include "printtm.h"

char *program_name = "getft";

static void
usage (int status)
{
  fputs ("Usage: getft [OPTION]... FILE\n", stdout);
# ifdef USE_TM_CYGWIN
  fputs ("\
Display FILE's time in seconds since 1970-01-01 00:00 UTC if exists,\n\
otherwise, \"-1\".\n\
", stdout);
# else
  fputs ("\
Display FILE's time in 100 nanoseconds since 1601-01-01 00:00 UTC\n\
if exists, otherwise, \"0\".\n\
", stdout);
# endif
  fputs ("\
\n\
Options:\n\
  -a   output the last access time\n\
", stdout);
# ifndef USE_TM_CYGWIN
  fputs ("\
  -b   output the creation time\n\
  -E   output time in seconds since 1970-01-01 00:00 UTC\n\
", stdout);
# else
  fputs ("\
  -F   output time in 100 nanoseconds since 1601-01-01 00:00 UTC\n\
  -h   output time of symbolic link instead of referenced file\n\
", stdout);
# endif
  fputs ("\
  -m   output the last write time (by default)\n\
  -n   don't output the trailing newline\n\
", stdout);
  exit (status);
}

# ifndef USE_TM_CYGWIN
LPWSTR *wargv = NULL;
# endif

int
main (int argc, char **argv)
{
  struct tmout_fmt ft_fmt = { false };
  struct tmout_ptrs ft_ptrs = { NULL };
  struct file ft_file;
  FT ft[FT_SIZE];
  FT *ftp = ft + FT_MTIME;
  intmax_t ft_elapse = 0;
  int ft_frac = 0;
  int c;
  bool success;
  bool seconds_set = false;

# ifdef USE_TM_CYGWIN
  seconds_set = true;
  ft_elapse = -1;
  ft_file.no_dereference = false;
# endif
  ft_ptrs.tm_elapse = &ft_elapse;

  while ((c = getopt (argc, argv, ":abEFhmn")) != -1)
    {
      switch (c)
        {
        case 'a':
          ftp = ft + FT_ATIME;
          break;
# ifndef USE_TM_CYGWIN
        case 'b':
          ftp = ft + FT_CTIME;
          break;
        case 'E':
          seconds_set = true;
          ft_elapse = -1;
          break;
# else
        case 'F':
          seconds_set = false;
          ft_elapse = 0;
          break;
        case 'h':
          ft_file.no_dereference = true;
          break;
# endif
        case 'm':
          ftp = ft + FT_MTIME;
          break;
        case 'n':
          ft_fmt.no_newline = true;
          break;
        default:
          usage (EXIT_FAILURE);
        }
    }

  if (argc <= optind || argc - 1 > optind)
    usage (EXIT_FAILURE);

# ifndef USE_TM_CYGWIN
  int wargc;
  wargv = CommandLineToArgvW (GetCommandLineW (), &wargc);
  if (wargv == NULL)
    error (EXIT_FAILURE, ERRNO (), "failed to get command arguments");

  ft_file.name = wargv[optind];
# else
  ft_file.name = argv[optind];
# endif

  success = getft (ft, &ft_file);

  if (success)
    {
      if (seconds_set)  /* Seconds since Unix epoch */
        {
          success = ft2secns (ftp, &ft_elapse, &ft_frac);
          if (success)
            ft_ptrs.tm_frac = &ft_frac;
        }
      else  /* 100 nanoseconds since 1601-01-01 00:00 UTC */
        ft_elapse = toftval (ftp);
    }

  printtm (&ft_fmt, &ft_ptrs);

# ifndef USE_TM_CYGWIN
  LocalFree (wargv);
# endif

  return success ? EXIT_SUCCESS : EXIT_FAILURE;
}
#endif
