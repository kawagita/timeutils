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

#ifdef USE_TM_CYGWIN
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
# include <stdio.h>
# include <unistd.h>

# include "cmdtmio.h"
# include "errft.h"
# include "error.h"
# include "exit.h"
# include "modifysec.h"

char *program_name = "getft";

static void
usage (int status)
{
  printusage ("getft", " FILE\n\
Display FILE's time " IN_DEFAULT_TIME ".\n\
\n\
Options:\n\
  -a        output the last access time\n\
  -A        arrange digits of nanoseconds at random\n"
# ifndef USE_TM_CYGWIN
"\
  -b        output the creation time\n\
  -C        round up to the smallest second that is not less than time\n\
  -F        round down to the largest second that does not exceed time\n"
# else
"\
  -C        round up to the smallest second that is not less than time\n\
  -f        output time " IN_FILETIME "\n\
  -F        round down to the largest second that does not exceed time\n\
  -h        output time of symbolic link instead of referenced file\n"
# endif
"\
  -m        output the last write time (by default)\n"
# ifndef USE_TM_CYGWIN
"\
  -s        output time " IN_UNIX_SECONDS "\n"
# endif
"\
  -R SEED   set nanoseconds at random by SEED; If 0, use current time\
", true, 0);
  exit (status);
}

# ifndef USE_TM_CYGWIN
LPWSTR *wargv = NULL;
# endif

int
main (int argc, char **argv)
{
  struct file ft_file;
  FT ft[FT_SIZE];
  FT *ftp = ft + FT_MTIME;
  intmax_t ft_elapse = 0;
  int ft_frac_val = 0;
  int ft_modflag = 0;
  int seed = 0;
  int c;
  int set_num;
  char *endptr;
  bool success = true;
  bool seconds_output = false;
  struct tm_fmt ft_fmt = { false };
  struct tm_ptrs ft_ptrs = (struct tm_ptrs) { .elapse = &ft_elapse };

# ifdef USE_TM_CYGWIN
  seconds_output = true;
  ft_elapse = -1;
  ft_file.no_dereference = false;
# endif

  while ((c = getopt (argc, argv, ":aAbCfFhmsR:")) != -1)
    {
      switch (c)
        {
        case 'a':
          ftp = ft + FT_ATIME;
          break;
        case 'A':
          ft_modflag |= NSEC_ARRANGE;
          break;
# ifndef USE_TM_CYGWIN
        case 'b':
          ftp = ft + FT_CTIME;
          break;
        case 's':
          seconds_output = true;
          ft_elapse = -1;
          break;
# else
        case 'f':
          seconds_output = false;
          ft_elapse = 0;
          break;
        case 'h':
          ft_file.no_dereference = true;
          break;
# endif
        case 'C':
          ft_modflag |= SECONDS_ROUND_UP;
          break;
        case 'F':
          ft_modflag |= SECONDS_ROUND_DOWN;
          break;
        case 'm':
          ftp = ft + FT_MTIME;
          break;
        case 'R':
          ft_modflag |= NSEC_RANDOM;
          set_num = sscannumuint (optarg, &seed, &endptr);
          if (set_num < 0)
            error (EXIT_FAILURE, 0, "invalid seed value %s", optarg);
          else if (set_num == 0 || *endptr != '\0')
            usage (EXIT_FAILURE);
          break;
        default:
          usage (EXIT_FAILURE);
        }
    }

  if (SECONDS_ROUNDING (ft_modflag) == (SECONDS_ROUND_DOWN | SECONDS_ROUND_UP))
    error (EXIT_FAILURE, 0, "cannot specify the both of rounding down and up");
  else if (argc <= optind || argc - 1 > optind)
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

  if (! getft (ft, &ft_file))
    errfile (EXIT_FAILURE, ERRNO (), "failed to get attributes of ", &ft_file);

  /* Generate a new sequence at once before get random values. */
  if (ft_modflag & (NSEC_RANDOM | NSEC_ARRANGE))
    srandsec (--seed);

  if (seconds_output)  /* Seconds since Unix epoch */
    {
      success = ft2sec (ftp, &ft_elapse, &ft_frac_val);

      if (success)
        {
          if (ft_modflag)
            success = modifysec (&ft_elapse, &ft_frac_val, ft_modflag);

          /* If a time is overflow for time_t of 32 bits, output "-1". */
          if (success)
            ft_ptrs.frac_val = &ft_frac_val;
        }
      else
        ft_elapse = -1;
    }
  else  /* 100 nanoseconds since 1601-01-01 00:00 UTC */
    ft_elapse = toftval (ftp, ft_modflag);

  printtm (&ft_fmt, &ft_ptrs);

# ifndef USE_TM_CYGWIN
  LocalFree (wargv);
# endif

  return success ? EXIT_SUCCESS : EXIT_FAILURE;
}
#endif
