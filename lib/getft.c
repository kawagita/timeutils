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

#ifdef USE_TM_GLIBC
# include <sys/stat.h>
# include <time.h>
#else
# ifndef UNICODE
#  define UNICODE
# endif
# include <windows.h>
#endif
#include <stdbool.h>
#include <stdint.h>

#include "ft.h"

/* Get file times for the specified struct file into FT and set the flag
   of a directory into the isdir member in *FT_FILE. If the no_dereference
   member is true, get the time of symbolic link but not a file referenced
   by it. Return true if successfull, otherwise, false.  */

bool
getft (FT ft[FT_SIZE], struct file *ft_file)
{
#ifdef USE_TM_GLIBC
  struct stat st;

  if ((ft_file->no_dereference
       ? lstat (ft_file->name, &st) : stat (ft_file->name, &st)) == 0)
    {
      ft[FT_ATIME] = st.st_atim;
      ft[FT_MTIME] = st.st_mtim;

      ft_file->isdir = S_ISDIR (st.st_mode);

      return true;
    }
#else
  WIN32_FILE_ATTRIBUTE_DATA finfo;

  if (GetFileAttributesEx (ft_file->name, GetFileExInfoStandard, &finfo))
    {
      ft[FT_ATIME] = finfo.ftLastAccessTime;
      ft[FT_MTIME] = finfo.ftLastWriteTime;
      ft[FT_BTIME] = finfo.ftCreationTime;

      ft_file->isdir = finfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY != 0;

      return true;
    }
#endif

  return false;
}

#ifdef TEST
# include <stdlib.h>
# include <string.h>
# include <unistd.h>

# include "argempty.h"
# include "argnum.h"
# include "cmdtmio.h"
# include "errft.h"
# include "error.h"
# include "exit.h"
# include "ftsec.h"

char *program_name = "getft";

static void
usage (int status)
{
  printusage ("getft", " FILE\n\
Display FILE's time " IN_DEFAULT_TIME ".\n\
\n\
Options:\n"
# ifdef USE_TM_GLIBC
"\
  -a        output the access time\n"
# else
"\
  -a        output the last access time\n\
  -b        output the creation time\n"
# endif
"\
  -C        round up to the smallest second that is not less than time\n\
  -F        round down to the largest second that does not exceed time\n"
# ifdef USE_TM_GLIBC
"\
  -h        output time of symbolic link instead of referenced file\n\
  -m        output the modification time (by default)\n"
# else
"\
  -m        output the last write time (by default)\n"
# endif
"\
  -P        permute digits in nanoseconds less than a second at random\n\
  -R SEED   change nanoseconds at random by SEED; If 0, use current time\n"
# ifdef USE_TM_GLIBC
"\
  -v        output time " IN_FILETIME
# else
"\
  -s        output time " IN_UNIX_SECONDS
# endif
, true, false, 0);
  exit (status);
}

# ifndef USE_TM_GLIBC
LPWSTR *wargv = NULL;
# endif

int
main (int argc, char **argv)
{
  struct file ft_file;
  FT ft[FT_SIZE];
  FT *ftp = ft + FT_MTIME;
  intmax_t ft_elapse = 0;
  int ft_frac_val = -1;
  int modflag = 0;
  int seed = 0;
  int c;
  int set_num;
  char *endptr;
  bool success = true;
  bool seconds_output = false;
# ifdef USE_TM_GLIBC
  bool no_dereference = false;

  seconds_output = true;
  ft_elapse = -1;
# endif

  while ((c = getopt (argc, argv, ":abCFhmPR:sv")) != -1)
    {
      switch (c)
        {
        case 'a':
          ftp = ft + FT_ATIME;
          break;
# ifndef USE_TM_GLIBC
        case 'b':
          ftp = ft + FT_BTIME;
          break;
        case 's':
          seconds_output = true;
          ft_elapse = -1;
          break;
# else
        case 'h':
          no_dereference = true;
          break;
        case 'v':
          seconds_output = false;
          ft_elapse = 0;
          break;
# endif
        case 'C':
          modflag |= FT_SECONDS_ROUND_UP;
          break;
        case 'F':
          modflag |= FT_SECONDS_ROUND_DOWN;
          break;
        case 'm':
          ftp = ft + FT_MTIME;
          break;
        case 'P':
          modflag |= FT_NSEC_PERMUTE;
          break;
        case 'R':
          set_num = argnumuint (optarg, &seed, &endptr);
          if (set_num < 0)
            error (EXIT_FAILURE, 0, "invalid seed value '%s'", optarg);
          else if (set_num == 0 || ! argempty (endptr))
            usage (EXIT_FAILURE);
          modflag |= FT_NSEC_RANDOM;
          break;
        default:
          usage (EXIT_FAILURE);
        }
    }

  if (IS_FT_SECONDS_ROUND_UP (modflag) && IS_FT_SECONDS_ROUND_DOWN (modflag))
    error (EXIT_FAILURE, 0, "cannot specify the both of rounding down and up");
  else if (argc <= optind || argc - 1 > optind)
    usage (EXIT_FAILURE);

# ifdef USE_TM_GLIBC
  INIT_FILE (ft_file, argv[optind], no_dereference);
# else
  int wargc;

  wargv = CommandLineToArgvW (GetCommandLineW (), &wargc);
  if (wargv == NULL)
    error (EXIT_FAILURE, ERRNO (), "failed to get command arguments");

  INIT_FILE (ft_file, wargv[optind], false);
# endif

  if (! getft (ft, &ft_file))
    errfile (EXIT_FAILURE, ERRNO (), "failed to get attributes of", &ft_file);

  /* Generate a new sequence at once before get random values. */
  if (IS_FT_NSEC_RANDOMIZING (modflag))
    srandsec (--seed);

  if (seconds_output)  /* Seconds since 1970-01-01 00:00 UTC */
    {
      int frac_val;

      success = ft2sec (ftp, &ft_elapse, &frac_val)
                && (!modflag || modifysec (&ft_elapse, &frac_val, modflag));

      if (success)
        ft_frac_val = frac_val;
      else
        ft_elapse = -1;
    }
  else  /* 100 nanoseconds since 1601-01-01 00:00 UTC */
    success = ft2val (ftp, modflag, &ft_elapse);

  printelapse (false, ft_elapse, ft_frac_val);

# ifndef USE_TM_GLIBC
  LocalFree (wargv);
# endif

  return success ? EXIT_SUCCESS : EXIT_FAILURE;
}
#endif
