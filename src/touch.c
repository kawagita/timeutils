/* touch -- change the creation, last access or write time of files on NTFS
   Copyright (C) 1987-2025 Free Software Foundation, Inc.
   Copyright (C) 2025 Yoshinori Kawagita.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* Written by Paul Rubin, Arnold Robbins, Jim Kingdon, David MacKenzie,
   and Randy Smith. */

#include "config.h"

#ifdef USE_TM_GLIBC
# include <string.h>
# include <time.h>
# include <unistd.h>
# include "fd-reopen.h"
# include "utimens.h"
#else
# include <windows.h>
#endif
#include <getopt.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "argempty.h"
#include "argmatch.h"
#include "argnum.h"
#include "ft.h"
#include "ftsec.h"
#include "errft.h"
#include "error.h"
#include "exit.h"
#include "posixtm.h"
#include "wintm.h"

#define _(msgid) (msgid)

/* The official name of this program (e.g., no `g' prefix).  */
#define PROGRAM_NAME "touch"

/* The base version of this program.  */
#define BASE_VERSION "9.6"

/* Authors and the modifier of this program.  */
#define AUTHORS "Paul Rubin, Arnold Robbins, Jim Kingdon", \
"David MacKenzie", "Randy Smith"
#define MODIFIER "Yoshinori Kawagita"

/* Bitmasks for `change_times'. */
#define CH_ATIME 1
#define CH_MTIME 2
#define CH_BTIME 4

/* The name by which this program was run. */
char *program_name = "touch";

/* Which timestamps to change. */
static int change_times;

/* Time used for changing timestamps.  */
static int change_used_time;

/* (-c) If true, don't create if not already there.  */
static bool no_create;

/* (-r) If true, use times from a reference file.  */
static bool use_ref;

/* (-e) If true, use times from each file.  */
static bool use_each;

/* New access, modification, and creation times to use when setting time.  */
static FT newtime[FT_SIZE];

/* File to use for -r. */
static struct file ref_file;

/* If DST is in effect or not for a time that is either skipped over or
   repeated when a transition to or from DST occurs, specify a positive
   value or zero, otherwise, attempt to determine whether the specified
   time is included in the term of DST.  */
int trans_isdst = 1;

/* For long options that have no equivalent short option, use a
   non-character as a pseudo short option, starting with CHAR_MAX + 1.  */
enum
{
  TIME_OPTION = 256,
  NS_PERMUTE_OPTION,
  NS_RANDOM_OPTION,
  ROUND_DOWN_OPTION,
  ROUND_UP_OPTION,
  TRANS_NODST_OPTION,
  HELP_OPTION,
  VERSION_OPTION
};

static struct option const longopts[] =
{
  {"time", required_argument, NULL, TIME_OPTION},
  {"no-create", no_argument, NULL, 'c'},
  {"date", required_argument, NULL, 'd'},
  {"reference", required_argument, NULL, 'r'},
  {"reference-each", no_argument, NULL, 'e'},
#ifdef USE_TM_GLIBC
  {"no-dereference", no_argument, NULL, 'h'},
#else
  {"use-btime", no_argument, NULL, 'C'},
#endif
  {"use-atime", no_argument, NULL, 'A'},
  {"use-mtime", no_argument, NULL, 'M'},
  {"ns-permute", no_argument, NULL, NS_PERMUTE_OPTION},
  {"ns-random", required_argument, NULL, NS_RANDOM_OPTION},
  {"round-down", no_argument, NULL, ROUND_DOWN_OPTION},
  {"round-up", no_argument, NULL, ROUND_UP_OPTION},
#ifdef USE_TM_SELFIMPL
  {"trans-nodst", no_argument, NULL, TRANS_NODST_OPTION},
#endif
  {"help", no_argument, NULL, HELP_OPTION},
  {"version", no_argument, NULL, VERSION_OPTION},
  {NULL, 0, NULL, 0}
};

/* The table of valid arguments to the `--time' option and the bits
   in `change_times' that those arguments set. */
static struct arg_table const time_args[] =
{
  { "atime", CH_ATIME }, { "access", CH_ATIME }, { "use", CH_ATIME },
  { "mtime", CH_MTIME },
#ifndef USE_TM_GLIBC
  { "write", CH_MTIME },
#endif
  { "modify", CH_MTIME },
#ifndef USE_TM_GLIBC
  { "btime", CH_BTIME }, { "creation", CH_BTIME }, { "birth", CH_BTIME },
#endif
  { NULL, -1 }
};

/* Update the time of file FILE according to the options given.
   Return true if successful.  */

static bool
touch (struct file *ft_file, const FT_CHANGE *ft_chg, bool date_set)
{
  const FT *ft_nowp[FT_SIZE];
  FT ft[FT_SIZE];
  int open_errno = 0;
  int set_errno = 0;

  if (IS_FILE_STDOUT (ft_file) || getft (ft, ft_file)
      || (open_errno = ERRNO (), ERRFILE_NOT_FOUND (open_errno)))
    {
      int i;

#ifdef USE_TM_GLIBC
      if (! no_create)
        {
#endif
          /* Try to open FILE, creating it if necessary.  */
          OPEN_FILE (ft_file, no_create);

          if (IS_INVALID_FILE (ft_file, true))
            open_errno = ERRNO ();
#ifdef USE_TM_GLIBC
        }
      else
        open_errno = 0;
#endif

      for (i = 0; i < FT_SIZE; i++)
        {
          if (change_times & (1 << i))
            {
              FT *ftp = newtime;

              if (!date_set)
                /* Use the access, modification, or creation time, or each
                   time of a file, instead of current time. */
                ftp = ft;

              ft_nowp[i] = ftp + (change_used_time < 0 ? i : change_used_time);
            }
          else
            ft_nowp[i] = NULL;
        }

      if (! setft (ft_file, ft_nowp, ft_chg))
        {
          set_errno = ERRNO ();

          if (!set_errno)
            {
              errfile (0, 0, _("date overflow for"), ft_file);
              return false;
            }
        }
    }
  else
    set_errno = open_errno;

#ifdef USE_TM_GLIBC
  if (ft_file->fd == STDIN_FILENO)
    {
      if (close (STDIN_FILENO) != 0)
        {
          errfile (0, ERRNO (), _("failed to close"), ft_file);
          return false;
        }
    }
  else if (ft_file->fd == STDOUT_FILENO)
    {
      /* Do not diagnose "touch -c - >&-".  */
      if (set_errno == EBADF && no_create)
        return true;
    }
#else
  CloseHandle (ft_file->hFile);
#endif

  if (set_errno != 0)
    {
      /* Don't diagnose with open_errno if FILE is a directory, as that
         would give a bogus diagnostic for e.g., 'touch /' (assuming we
         don't own / or have write access).  On Solaris 10 and probably
         other systems, opening a directory like "." fails with EINVAL.
         (On SunOS 4 it was EPERM but that's obsolete.)  */
      if (open_errno && ERRFILE_NOT_WRITTEN (open_errno, ft_file))
        {
          /* The wording of this diagnostic should cover at least two cases:
             - the file does not exist, but the parent directory is unwritable
             - the file exists, but it isn't writable
             I think it's not worth trying to distinguish them.  */
          errfile (0, open_errno, _("cannot touch"), ft_file);
        }
      else
        {
          if (no_create && ERRFILE_NOT_FOUND (set_errno))
            return true;
          errfile (0, set_errno, _("setting times of"), ft_file);
        }
      return false;
    }

  return true;
}

static void
usage (int status)
{
  if (status != EXIT_SUCCESS)
    fprintf (stderr, _("Try `%s --help' for more information.\n"),
             program_name);
  else
    {
      printf (_("Usage: %s [OPTION]... FILE...\n"), program_name);
#ifdef USE_TM_GLIBC
      fputs (_("\
Update the access and modification times of each FILE to the current time.\n\
\n\
A FILE argument that does not exist is created empty, unless -c or -h\n\
is supplied.\n\
\n\
A FILE argument string of - is handled specially and causes touch to\n\
change the times of the file associated with standard output.\n\
"), stdout);
#else
      fputs (_("\
Update the creation, last access and write times of each FILE to the current\n\
time on NTFS filesystem.\n\
\n\
A FILE argument that does not exist is created empty, unless -c is supplied.\n\
"), stdout);
#endif
      fputs (_("\
\n\
Mandatory arguments to long options are mandatory for short options too.\n\
\n\
"), stdout);
#ifdef USE_TM_GLIBC
      fputs (_("\
  -a                     change only the access time\n\
  -A, --use-atime        use the access time instead of current time\n\
"), stdout);
#else
      fputs (_("\
  -a                     change only the last access time\n\
  -A, --use-atime        use the last access time instead of current time\n\
  -b                     change only the creation time\n\
  -B, --use-btime        use the creation time instead of current time\n\
"), stdout);
#endif
      fputs (_("\
  -c, --no-create        do not create any files\n\
  -d, --date=STRING      parse STRING and use it instead of current time\n\
  -e, --reference-each   use each file's times instead of current time\n\
  -f                     (ignored)\n\
"), stdout);
#ifdef USE_TM_GLIBC
      fputs (_("\
  -h, --no-dereference   affect each symbolic link instead of any referenced\n\
                         file (useful only on systems that can change the\n\
                         timestamps of a symlink)\n\
  -m                     change only the modification time\n\
  -M, --use-mtime        use the modification time instead of current time\n\
"), stdout);
#else
      fputs (_("\
  -m                     change only the last write time\n\
  -M, --use-mtime        use the last write time instead of current time\n\
"), stdout);
#endif
      fputs (_("\
      --ns-permute       permute digits in nanoseconds at random\n\
      --ns-random=SEED   set the random value into nanoseconds by SEED;\n\
                         If 0, randomize by current time\n\
  -r, --reference=FILE   use this file's times instead of current time\n\
      --round-down       round down to the largest second that does not\n\
                         exceed file time\n\
      --round-up         round up to the smallest second that is not less\n\
                         than file time\n\
  -t [[CC]YY]MMDDhhmm[.ss]  use specified time instead of current time,\n\
                         with a date-time format that differs from -d's\n\
"), stdout);
#ifdef USE_TM_SELFIMPL
      fputs (_("\
  -T, --trans-nodst      do not adjust time by DST offset for a time that\n\
                         is skipped over and repeated in transition date\n\
"), stdout);
#endif
#ifdef USE_TM_GLIBC
      fputs (_("\
      --time=WORD        specify which time to change:\n\
                           access time (-a): 'access', 'atime', 'use';\n\
                           modification time (-m): 'modify', 'mtime'\n\
"), stdout);
#else
      fputs (_("\
      --time=WORD        specify which time to change:\n\
                           last access time (-a): 'access', 'atime', 'use';\n\
                           last write time (-m): 'write', 'modify', 'mtime';\n\
                           creation time (-b): 'creation', 'btime', 'birth'\n\
"), stdout);
#endif
      fputs (_("\
      --help             display this help and exit\n\
      --version          output version information and exit\n\
\n\
Note that the -d and -t options accept different time-date formats.\n\
"), stdout);
    }
  exit (status);
}

#ifndef USE_TM_GLIBC
LPWSTR *wargv = NULL;
#endif

static void
version (void)
{
  printf (_("\
%s %s Modified for Windows (%s %s)\n\
Copyright (C) 2025 Free Software Foundation, Inc.\n\
Copyright (C) 2025 %s.\n\
"), PROGRAM_NAME, BASE_VERSION, PACKAGE_NAME, PACKAGE_VERSION, MODIFIER);
      fputs (_("\
License GPLv3+: GNU GPL version 3 or later <https://gnu.org/licenses/gpl.html>.\n\
This is free software: you are free to change and redistribute it.\n\
There is NO WARRANTY, to the extent permitted by law.\n\
\n\
"), stdout);
  printf (_("\
%s (GNU coreutils) %s\n\
Written by %s,\n%s, and %s.\n\
"), PROGRAM_NAME, BASE_VERSION, AUTHORS);
  exit (0);
}

static void
unkopt (char *opt)
{
  fprintf (stderr, _("%s: unkonwn option -- "), PROGRAM_NAME);
  if (optopt)
    fputc (optopt, stderr);
  else
    {
      char *p = opt + 2;
      while (*p != '\0')
        {
          if (*p == '=')
            break;
          p++;
        }
      *p = '\0';
      fputs (opt + 2, stderr);
    }
  fputc ('\n', stderr);
  usage (EXIT_FAILURE);
}

int
main (int argc, char **argv)
{
  FT_PARSING ft_parsing;
  FT_CHANGE *ft_chgp = NULL;
  int used_times = 0;
  int seed = 0;
  int c, i;
  int set_num;
  bool date_set = false;
  bool ok = true;
#ifdef USE_TM_GLIBC
  bool no_dereference = false;
#endif
  char const *flex_date = NULL;
  char *endptr;

#ifndef USE_TM_GLIBC
  int wargc;

  wargv = CommandLineToArgvW (GetCommandLineW (), &wargc);
  if (wargv == NULL || wargc < argc)
    error (EXIT_FAILURE, ERRNO (), _("failed to get command arguments"));
#endif

  ft_parsing.change = (FT_CHANGE) { .datetime_unset = true, .modflag = 0 };
  change_times = 0;
  change_used_time = -1;
  no_create = use_ref = use_each = false;

  while ((c = getopt_long (argc, argv, ":aAbBcd:efmMr:t:", longopts, NULL)) != -1)
    {
      switch (c)
        {
        case 'a':
          change_times |= CH_ATIME;
          break;

        case 'A':
          change_used_time = FT_ATIME;
          used_times |= CH_ATIME;
          break;

#ifndef USE_TM_GLIBC
        case 'b':
          change_times |= CH_BTIME;
          break;

        case 'B':
          change_used_time = FT_BTIME;
          used_times |= CH_BTIME;
          break;
#endif

        case 'c':
          no_create = true;
          break;

        case 'd':
          if (! parseft (&ft_parsing, optarg))
            error (EXIT_FAILURE, 0, _("invalid date format '%s'"), optarg);
          else if (ft_parsing.timespec_seen)
            newtime[0] = ft_parsing.timespec.ft;
          else
            ft_chgp = &ft_parsing.change;
          flex_date = optarg;
          break;

        case 'e':
	  use_each = true;
          break;

        case 'f':
          break;

#ifdef USE_TM_GLIBC
        case 'h':
          no_dereference = true;
          break;
#endif
        case 'm':
          change_times |= CH_MTIME;
          break;

        case 'M':
          change_used_time = FT_MTIME;
          used_times |= CH_MTIME;
          break;

        case NS_PERMUTE_OPTION:	/* --ns-permute */
          ft_chgp = &ft_parsing.change;
          ft_chgp->modflag |= FT_NSEC_PERMUTE;
          break;

        case NS_RANDOM_OPTION:	/* --ns-random */
          set_num = argnumuint (optarg, &seed, &endptr);
          if (set_num <= 0 || ! argempty (endptr))
            {
              error (0, 0, _("invalid seed value '%s'"), optarg);
              usage (EXIT_FAILURE);
            }
          ft_chgp = &ft_parsing.change;
          ft_chgp->modflag |= FT_NSEC_RANDOM;
          break;

        case 'r':
#ifdef USE_TM_GLIBC
          INIT_FILE (ref_file, optarg, no_dereference);
#else
          INIT_FILE (ref_file, wargv[optind - 1], false);
#endif
	  use_ref = true;
          break;

        case ROUND_DOWN_OPTION:	/* --round-down */
          ft_chgp = &ft_parsing.change;
          ft_chgp->modflag |= FT_SECONDS_ROUND_DOWN;
          break;

        case ROUND_UP_OPTION:	/* --round-up */
          ft_chgp = &ft_parsing.change;
          ft_chgp->modflag |= FT_SECONDS_ROUND_UP;
          break;

        case 't':
          {
            intmax_t seconds;
            if (! posixtime (&seconds, optarg,
                             PDS_LEADING_YEAR | PDS_CENTURY | PDS_SECONDS)
                || ! sec2ft (seconds, 0, &newtime[0]))
              error (EXIT_FAILURE, 0, _("invalid date format '%s'"), optarg);
            for (i = 1; i < FT_SIZE; i++)
              newtime[i] = newtime[0];
          }
          date_set = true;
          break;

#ifdef USE_TM_SELFIMPL
        case TRANS_NODST_OPTION:	/* --trans-nodst */
          trans_isdst = 0;
          break;
#endif

        case TIME_OPTION:	/* --time */
          {
            int change_time;
            if (! argmatch (optarg, time_args, 0, &change_time, &endptr)
                || ! argempty (endptr))
              {
                error (0, 0, _("invalid argument '%s' for '--time'"), optarg);
                argmatch_valid (time_args);
                usage (EXIT_FAILURE);
              }
            change_times |= change_time;
          }
          break;

        case HELP_OPTION:	/* --help */
          usage (EXIT_SUCCESS);

        case VERSION_OPTION:	/* --version */
          version ();

        default:
          unkopt (argv[optind - 1]);
        }
    }

  if (change_times == 0)
    change_times = CH_ATIME | CH_MTIME | CH_BTIME;

  if ((date_set && (use_ref || use_each || used_times || flex_date))
      || (use_ref && use_each)
      || used_times >= (CH_ATIME | CH_MTIME))
    {
      error (0, 0, _("cannot specify times from more than one source"));
      usage (EXIT_FAILURE);
    }
  else if (ft_chgp)
    {
      if (IS_FT_SECONDS_ROUND_UP (ft_chgp->modflag)
          && IS_FT_SECONDS_ROUND_DOWN (ft_chgp->modflag))
        {
          error (0, 0, _("cannot specify the both of rounding down and up"));
          usage (EXIT_FAILURE);
        }
      else if (IS_FT_NSEC_RANDOMIZING (ft_chgp->modflag))
        /* Generate a new sequence at once before get random values. */
        srandsec (--seed);
    }

  /* Use each file's time if specify -A, -B, or -M without -r option. */
  if (!use_ref && change_used_time >= 0)
    use_each = true;

  /* Set the same new time for all files except the case that the access,
     modification, and/or creation time of each file is used. */
  if (use_ref || (!use_each && !date_set))
    {
      int date_set_index = change_used_time < 0 ? 0 : change_used_time;
      int date_end_index = date_set_index + FT_SIZE;
      i = date_set_index;

      if (use_ref)
        {
          if (! getft (newtime, &ref_file))
            errfile (EXIT_FAILURE, ERRNO (),
                     _("failed to get attributes of"), &ref_file);
          else if (change_used_time < 0)
            /* Use each time of a file for changing file time. */
            date_set_index = FT_SIZE;
        }
      else if (!flex_date || !ft_parsing.timespec_seen)
        {
          /* Set the new time to current time if not specified by
             "@ seconds" with -d option. */
#ifdef USE_TM_GLIBC
          if (!ft_chgp)
            /* Never change current time in below loop. */
            newtime[date_set_index].tv_nsec = UTIME_NOW;
          else
#endif
          if (! currentft (newtime + date_set_index))
            error (EXIT_FAILURE, 0, _("failed to get system clock"));
        }

      /* A new time is changed by -d or modification options firstly
         and then copied to others if --use-atime, --use-mtime, or
         --use-btime with -r option, otherwise, each time is changed. */
      do
        {
          if (i > date_set_index)
            newtime[i % FT_SIZE] = newtime[date_set_index];
          else if (ft_chgp && ! calcft (newtime + i, newtime + i, ft_chgp))
            {
              if (flex_date)
                error (EXIT_FAILURE, 0,
                       _("invalid date format '%s'"), flex_date);
              else
                error (EXIT_FAILURE, 0, _("cannot modify new time"));
            }
        }
      while (++i < date_end_index);

      ft_chgp = NULL;
      date_set = true;
    }
  else if (date_set && ft_chgp)
    {
      /* Change current time by modification options. */
      if (! calcft (newtime, newtime, ft_chgp))
        error (EXIT_FAILURE, 0, _("cannot modify current time"));

      for (i = 1; i < FT_SIZE; i++)
        newtime[i] = newtime[0];

      ft_chgp = NULL;
    }

  if (optind == argc)
    {
      error (0, 0, _("missing file operand"));
      usage (EXIT_FAILURE);
    }

  for (; optind < argc; ++optind)
    {
      struct file ft_file;
#ifdef USE_TM_GLIBC
      INIT_FILE (ft_file, argv[optind], no_dereference);
#else
      INIT_FILE (ft_file, wargv[optind], false);
#endif

      ok &= touch (&ft_file, ft_chgp, date_set);
    }

#ifndef USE_TM_GLIBC
  LocalFree (wargv);
#endif

  return (ok ? EXIT_SUCCESS : EXIT_FAILURE);
}
