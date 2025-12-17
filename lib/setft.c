/* Set the file time to parameters of time.

   Copyright (C) 199-2000, 2002-2025 Free Software Foundation, Inc.
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

/* Originally written by Steven M. Bellovin <smb@research.att.com> while
   at the University of North Carolina at Chapel Hill.  Later tweaked by
   a couple of people on Usenet.  Completely overhauled by Rich $alz
   <rsalz@bbn.com> and Jim Berets <jberets@bbn.com> in August, 1990.

   Modified by Paul Eggert <eggert@twinsun.com> in 1999 to do the
   right thing about local DST.  Also modified by Paul Eggert
   <eggert@cs.ucla.edu> in 2004 to support nanosecond-resolution
   timestamps, and in 2017 and 2020 to check for integer overflow
   and to support longer-than-'long' 'time_t' and 'tv_nsec'.  */

#include "config.h"

#ifdef USE_TM_GLIBC
# include <fcntl.h>
# include <string.h>
# include <sys/stat.h>
# include <time.h>
# include <unistd.h>
# include "utimens.h"
#else
# ifndef UNICODE
#  define UNICODE
# endif
# include <windows.h>
#endif
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "ft.h"
#include "ftsec.h"
#include "imaxoverflow.h"
#include "intoverflow.h"
#include "wintm.h"

#define EPOCH_YEAR 1970

/* If *TM0 is the old and *TM1 is the new value of a struct tm after
   passing it to mktime_z, return true if it's OK.  It's not OK if
   mktime failed or if *TM0 has out-of-range mainline members.
   The caller should set TM1->tm_wday to -1 before calling mktime,
   as a negative tm_wday is how mktime failure is inferred.  */

static bool
mktime_ok (TM const *tm0, TM const *tm1)
{
  if (tm1->tm_wday < 0)
    return false;

  return ! ((tm0->tm_sec ^ tm1->tm_sec)
            | (tm0->tm_min ^ tm1->tm_min)
            | (tm0->tm_hour ^ tm1->tm_hour)
            | (tm0->tm_mday ^ tm1->tm_mday)
            | (tm0->tm_mon ^ tm1->tm_mon)
            | (tm0->tm_year ^ tm1->tm_year));
}

/* Calculate file time for *NOW by members in *TM_CHG and set its value
   into *FT. If the datetime_unset member is true, don't calculate and copy
   *NOW changed by the modflag member to *FT. Return true if not overflow,
   otherwise, false.  */

bool
calcft (FT *ft, const FT *now, const FT_CHANGE *ft_chg)
{
  intmax_t Start;
  int Start_ns;

  if (! ft2sec (now, &Start, &Start_ns))
    return false;
  else if (ft_chg->datetime_unset)
    {
      if (ft_chg->modflag)
        {
          if (! modifysec (&Start, &Start_ns, ft_chg->modflag))
            return false;

          sec2ft (Start, Start_ns, ft);
        }
      else
        *ft = *now;

      return true;
    }
  else if (ft_chg->ns >= 0)
    Start_ns = ft_chg->ns;

  /* Never use the environment variable of a time zone ('TZ="XXX"').  */

  TM tm;
  TM tm0;

  if (! localtimew (&Start, &tm))
    return false;

  if (ft_chg->date_set)
    {
      if ((ft_chg->year >= 0
           && INT_SUBTRACT_WRAPV (ft_chg->year, TM_YEAR_BASE, &tm.tm_year))
          || INT_SUBTRACT_WRAPV (ft_chg->month, 1, &tm.tm_mon))
        return false;
      tm.tm_mday = ft_chg->day;
    }
  if (ft_chg->hour >= 0)
    {
      tm.tm_hour = ft_chg->hour;
      tm.tm_min = ft_chg->minutes;
      tm.tm_sec = ft_chg->seconds;
    }
  else if (! ft_chg->rel_set || ft_chg->date_set || ft_chg->day_number >= 0)
    tm.tm_hour = tm.tm_min = tm.tm_sec = Start_ns = 0;

  /* Let mktime deduce tm_isdst if we have an absolute timestamp.  */
  if (ft_chg->date_set || ft_chg->day_number >= 0 || ft_chg->hour >= 0)
    tm.tm_isdst = -1;

  /* But if the input explicitly specifies local time with or without
     DST, give mktime that information.  */
  if (ft_chg->lctz_isdst >= 0)
    tm.tm_isdst = ft_chg->lctz_isdst;

  int time_zone = ft_chg->tz_utcoff;

  tm0.tm_sec = tm.tm_sec;
  tm0.tm_min = tm.tm_min;
  tm0.tm_hour = tm.tm_hour;
  tm0.tm_mday = tm.tm_mday;
  tm0.tm_mon = tm.tm_mon;
  tm0.tm_year = tm.tm_year;
  tm0.tm_isdst = tm.tm_isdst;
  tm.tm_wday = -1;

  Start = mktimew (&tm);

  if (! mktime_ok (&tm0, &tm))
    {
      bool repaired = false;

      /* Guard against falsely reporting errors near the time_t boundaries
         when parsing times in other time zones.  For example, if the min
         time_t value is 1970-01-01 00:00:00 UTC and we are 8 hours ahead
         of UTC, then the min localtime value is 1970-01-01 08:00:00; if
         we apply mktime to 1970-01-01 00:00:00 we will get an error, so
         we apply mktime to 1970-01-02 08:00:00 instead and adjust the time
         zone by 24 hours to compensate.  This algorithm assumes that
         there is no DST transition within a day of the time_t boundaries.  */
      if (ft_chg->tz_set)
        {
          /* The below code was used to coreutils 5.2.1 in which the time
             zone is not set by an environment variable ('TZ="XXX"').  */
          tm = tm0;
          if (tm.tm_year <= EPOCH_YEAR - TM_YEAR_BASE)
            {
              tm.tm_mday++;
              if (INT_ADD_WRAPV (time_zone, 24 * 60, &time_zone))
                return false;
            }
          else
            {
              tm.tm_mday--;
              if (INT_SUBTRACT_WRAPV (time_zone, 24 * 60, &time_zone))
                return false;
            }

          tm.tm_sec = tm0.tm_sec;
          tm.tm_min = tm0.tm_min;
          tm.tm_hour = tm0.tm_hour;
          tm.tm_mday = tm0.tm_mday;
          tm.tm_mon = tm0.tm_mon;
          tm.tm_year = tm0.tm_year;
          tm.tm_isdst = tm0.tm_isdst;
          tm.tm_wday = -1;
          Start = mktimew (&tm);
          repaired = mktime_ok (&tm0, &tm);
        }

      if (! repaired)
        return false;
    }

  if (ft_chg->day_number >= 0)
    {
      intmax_t dayincr;
      intmax_t day_ordinal = (ft_chg->day_ordinal
                              - (0 < ft_chg->day_ordinal
                                 && tm.tm_wday != ft_chg->day_number));
      tm.tm_yday = -1;

      if (! (IMAX_MULTIPLY_WRAPV (day_ordinal, 7, &dayincr)
             || IMAX_ADD_WRAPV ((ft_chg->day_number - tm.tm_wday + 7) % 7,
                                dayincr, &dayincr)
             || dayincr < INT_MIN || dayincr > INT_MAX
             || INT_ADD_WRAPV (dayincr, tm.tm_mday, &tm.tm_mday)))
        {
          tm.tm_isdst = -1;
          Start = mktimew (&tm);
        }

      if (tm.tm_yday < 0)
        return false;
    }

  /* Add relative date.  */
  if (ft_chg->rel_year | ft_chg->rel_month | ft_chg->rel_day)
    {
      int year, month, day;
      if (INT_ADD_WRAPV (tm.tm_year, ft_chg->rel_year, &year)
          || INT_ADD_WRAPV (tm.tm_mon, ft_chg->rel_month, &month)
          || INT_ADD_WRAPV (tm.tm_mday, ft_chg->rel_day, &day))
        return false;
      tm.tm_year = year;
      tm.tm_mon = month;
      tm.tm_mday = day;
      tm.tm_hour = tm0.tm_hour;
      tm.tm_min = tm0.tm_min;
      tm.tm_sec = tm0.tm_sec;
      tm.tm_isdst = tm0.tm_isdst;
      tm.tm_wday = -1;
      Start = mktimew (&tm);
      if (tm.tm_wday < 0)
        return false;
    }

  if (ft_chg->tz_set)
    {
      bool overflow = false;
      long int utcoff = tm.tm_gmtoff;
      intmax_t delta;
      intmax_t t1;
      overflow |= IMAX_SUBTRACT_WRAPV (ft_chg->tz_utcoff, utcoff, &delta);
      overflow |= IMAX_SUBTRACT_WRAPV (Start, delta, &t1);
      if (overflow)
        return false;
      Start = t1;
    }

  /* Add relative hours, minutes, and seconds.  On hosts that support
     leap seconds, ignore the possibility of leap seconds; e.g.,
     "+ 10 minutes" adds 600 seconds, even if one of them is a
     leap second.  Typically this is not what the user wants, but it's
     too hard to do it the other way, because the time zone indicator
     must be applied before relative times, and if mktime is applied
     again the time zone will be lost.  */
  intmax_t orig_ns = Start_ns;
  intmax_t sum_ns = orig_ns + ft_chg->rel_ns;
  int normalized_ns =
        (sum_ns % FT_NSEC_PRECISION + FT_NSEC_PRECISION) % FT_NSEC_PRECISION;
  int d4 = (sum_ns - normalized_ns) / FT_NSEC_PRECISION;
  intmax_t d1, t1, d2, t2, t3, t4;
  if (IMAX_MULTIPLY_WRAPV (ft_chg->rel_hour, 60 * 60, &d1)
      || IMAX_ADD_WRAPV (Start, d1, &t1)
      || IMAX_MULTIPLY_WRAPV (ft_chg->rel_minutes, 60, &d2)
      || IMAX_ADD_WRAPV (t1, d2, &t2)
      || IMAX_ADD_WRAPV (t2, ft_chg->rel_seconds, &t3)
      || IMAX_ADD_WRAPV (t3, d4, &t4)
      || (ft_chg->modflag
          && ! modifysec (&t4, &normalized_ns, ft_chg->modflag)))
    return false;

  return sec2ft (t4, normalized_ns, ft);
}

/* Change the specified file time by members in *TM_CHG and set its value
   to the file specified by *FT_FILE. If TM_CHG is NULL, copy directly it
   to the file, or if a pointer included in FT_NOWP is NULL, set its time
   to current time. Return true if successfull, otherwise, false.  */

bool
setft (struct file *ft_file, const FT *ft_nowp[FT_SIZE],
       const FT_CHANGE *ft_chg)
{
  if (! IS_INVALID_FILE (ft_file, false))
    {
      const FT *ftp[FT_SIZE];
      FT ft[FT_SIZE];
      int i;

      for (i = 0; i < FT_SIZE; i++)
        {
          if (ft_nowp[i])
            {
              if (ft_chg)
                {
                  if (! calcft (ft + i, ft_nowp[i], ft_chg))
                    return false;

#ifndef USE_TM_GLIBC
                  ftp[i] = ft + i;
#endif
                }
              else
                /* Copy the value pointed to FT_NOW[i] to file time. */
#ifdef USE_TM_GLIBC
                ft[i] = *ft_nowp[i];
#else
                ftp[i] = ft_nowp[i];
#endif
            }
          else
            /* Don't change a time by utimensat or SetFileTime function
               if the pointer is NULL. */
#ifdef USE_TM_GLIBC
            ft[i].tv_nsec = UTIME_OMIT;
#else
            ftp[i] = NULL;
#endif
        }

#ifdef USE_TM_GLIBC
      int fd = ft_file->fd;
      char const *file_opt = fd == STDOUT_FILENO ? NULL : ft_file->name;
      int atflag = ft_file->no_dereference ? AT_SYMLINK_NOFOLLOW : 0;

      if (fdutimensat (fd, AT_FDCWD, file_opt, ft, atflag) == 0)
#else
      if (SetFileTime (ft_file->hFile,
                       ftp[FT_BTIME], ftp[FT_ATIME], ftp[FT_MTIME]))
#endif
        return true;
    }

  return false;
}

#ifdef TEST
# include <stdio.h>
# include <unistd.h>

# include "argempty.h"
# include "argnum.h"
# include "cmdtmio.h"
# include "errft.h"
# include "error.h"
# include "exit.h"
# ifdef USE_TM_GLIBC
#  include "fd-reopen.h"
# endif

int trans_isdst = 1;

char *program_name = "setft";

static void
usage (int status)
{
  printusage ("setft", " FILE [WEEKDAY_NAME[,NUMBER]] DATETIME\n\
             [" DST_ST_NOTATION "] YEAR MONTH DAY HOUR MINUTE SECOND\
 NANOSECOND\n\
Calculate file time by adding YEAR, MONTH, DAY, HOUR, MINUTE, SECOND,\n\
and NANOSECOND to current time or by advancing its date to NUMBERth\n\
WEEKDAY_NAME. Set the calculated value into FILE's time if not specify\n\
\"-\", otherwise, display " IN_DEFAULT_TIME ".\n\
\n\
  [YYYY-MM-DD][Thh:mm:ss[" FT_NSEC_NOTATION "]][Z|+hhmm|-hhmm]\n\
\n\
Replace current time by any parameters of DATETIME in above ISO 8601\n\
format and adjust it by \"+hhmm\" or \"-hhmm\" for current time zone. But\n\
must specify \"-0001-00-00\" when no parameter is replaced.\
", true, true, 'T');
fputs ("\
\n\
Options:\n"
# ifdef USE_TM_GLIBC
"\
  -a        change the access time\n"
# else
"\
  -a        change the last access time\n\
  -b        change the creation time\n"
# endif
"\
  -C        round up to the smallest second that is not less than time\n\
  -F        round down to the largest second that does not exceed time\n"
# ifdef USE_TM_GLIBC
"\
  -h        change time of symbolic link instead of referenced file\n\
  -m        change the modification time (by default)\n"
# else
"\
  -m        change the last write time (by default)\n"
# endif
"\
  -P        permute digits in nanoseconds less than a second at random\n\
  -R SEED   change nanoseconds at random by SEED; If 0, use current time\n\
  -r FILE   use FILE's timestamp instead of current time\n"
# ifdef USE_TM_GLIBC
"\
  -v        output time " IN_FILETIME
# else
"\
  -s        output time " IN_UNIX_SECONDS
# endif
"\n", stdout);
  exit (status);
}

/* Parse the specified arguments as the date, time, and UTC offset in
   ISO 8601 format and set those values into *FT_CHGP, storing the pointer
   to a following character into *ENDPTR. Return 1 or 0 if set values or
   not, otherwise, -1 if a value is outside the range of its parameter.  */
static int
argdatetime (const char *argv, FT_CHANGE *ft_chgp, char **endptr)
{
  int *dates[] = { &ft_chgp->year, &ft_chgp->month, &ft_chgp->day };
  int *times[] = { &ft_chgp->hour, &ft_chgp->minutes, &ft_chgp->seconds };
  long int tz_utcoff;
  struct tm_ptrs ft_ptrs =
    (struct tm_ptrs) { .dates = dates, .times = times,
                       .ns = &ft_chgp->ns, .utcoff = &tz_utcoff };
  int set_num = argtmiso8601 (argv, &ft_ptrs, endptr);
  if (set_num <= 0)
    return set_num;

  /* Set the value of a month and day instead of current time to MM
     and DD in ISO 8601 format if the both is not zero, otherwise,
     don't set those values. */
  if (ft_chgp->month == 0)
    {
      if (ft_chgp->year >= 0 || ft_chgp->day)
      /* YYYY-00-00, or -0001-00-DD */
        return 0;
    }
  else if (ft_chgp->month)
    {
      if (ft_chgp->day == 0)  /* YYYY-MM-00 or -0001-MM-00 */
        return 0;
      ft_chgp->date_set = true;
    }

  if (ft_chgp->hour >= 0)
    {
      set_num -= 3;
      if (ft_chgp->ns >= 0)
        set_num--;
      else
        ft_chgp->ns = 0;
    }

  if (set_num % 3)
    {
      ft_chgp->tz_set = true;
      ft_chgp->tz_utcoff = tz_utcoff;
    }

  return 1;
}

# ifndef USE_TM_GLIBC
LPWSTR *wargv = NULL;
# endif

int
main (int argc, char **argv)
{
  struct file ft_file = { NULL };
  struct file ref_file = { NULL };
  const FT *ft_nowp[FT_SIZE];
  FT ft[FT_SIZE];
  FT now[FT_SIZE];
  FT_CHANGE ft_chg =
    (FT_CHANGE) { .datetime_unset = false, .date_set = false, .year = -1,
                  .hour = -1, .minutes = -1, .seconds = -1, .ns = -1,
                  .day_number = -1, .tz_set = false, .lctz_isdst = -1 };
  intmax_t ft_elapse = 0;
  int ft_frac_val = -1;
  int ftind = -1;
  int seed = 0;
  int c, i;
  int set_num;
  char *endptr;
  bool success = true;
  bool seconds_output = false;
# ifdef USE_TM_GLIBC
  bool no_dereference = false;

  seconds_output = true;
  ft_elapse = -1;
# else
  int wargc;

  wargv = CommandLineToArgvW (GetCommandLineW (), &wargc);
  if (wargv == NULL || wargc < argc)
    error (EXIT_FAILURE, ERRNO (), "failed to get command arguments");
# endif

  while ((c = getopt (argc, argv, ":abCFhmPr:R:svT")) != -1)
    {
      switch (c)
        {
        case 'a':
          ftind = FT_ATIME;
          break;
# ifndef USE_TM_GLIBC
        case 'b':
          ftind = FT_BTIME;
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
          ft_chg.modflag |= FT_SECONDS_ROUND_UP;
          break;
        case 'F':
          ft_chg.modflag |= FT_SECONDS_ROUND_DOWN;
          break;
        case 'm':
          ftind = FT_MTIME;
          break;
        case 'P':
          ft_chg.modflag |= FT_NSEC_PERMUTE;
          break;
        case 'R':
          set_num = argnumuint (optarg, &seed, &endptr);
          if (set_num < 0)
            error (EXIT_FAILURE, 0, "invalid seed value '%s'", optarg);
          else if (set_num == 0 || ! argempty (endptr))
            usage (EXIT_FAILURE);
          ft_chg.modflag |= FT_NSEC_RANDOM;
          break;
        case 'r':
# ifdef USE_TM_GLIBC
          INIT_FILE (ref_file, optarg, no_dereference);
# else
          INIT_FILE (ref_file, wargv[optind - 1], false);
# endif
          break;
# ifdef USE_TM_SELFIMPL
        case 'T':
          trans_isdst = 0;
          break;
# endif
        default:
          usage (EXIT_FAILURE);
        }
    }

  argc -= optind;
  argv += optind;

  if (IS_FT_SECONDS_ROUND_UP (ft_chg.modflag)
      && IS_FT_SECONDS_ROUND_DOWN (ft_chg.modflag))
    error (EXIT_FAILURE, 0, "cannot specify the both of rounding down and up");
  else if (argc < 2)
    usage (EXIT_FAILURE);

  /* Set the file name to the first argument after options if not "-". */
  if (strcmp (*argv, "-") != 0)
    {
      for (i = 0; i < FT_SIZE; i++)
        ft_nowp[i] = now + i;

# ifdef USE_TM_GLIBC
      INIT_FILE (ft_file, *argv, no_dereference);
# else
      INIT_FILE (ft_file, wargv[optind], false);
# endif
    }

  argc--;
  argv++;

  /* Attempt to set a week day from the second argument after options. */
  set_num = argweekday (*argv,
                        &ft_chg.day_number, &ft_chg.day_ordinal, &endptr);
  if (set_num < 0)
    error (EXIT_FAILURE, 0, "invalid week day ordinal '%s'", *argv);
  else if (set_num > 0)
    {
      if (*endptr != '\0' || --argc < 1)
        usage (EXIT_FAILURE);
      argv++;
    }

  /* Input parameters of date and time in ISO 8601 format from the next
     argument of the file name and/or week day, replaced for the current
     or file time. */
  set_num = argdatetime (*argv, &ft_chg, &endptr);
  if (set_num < 0)
    error (EXIT_FAILURE, 0, "invalid date time '%s'", *argv);
  else if (set_num == 0)
    usage (EXIT_FAILURE);

  argc--;
  argv++;

  /* Attempt to set the isdst flag from the rest of arguments. */
  if (argc > 0 && argisdst (*argv, &ft_chg.lctz_isdst))
    {
      argc--;
      argv++;
    }

  /* Set each parameter of relative time from the rest of arguments
     if values are specified. */
  if (argc > 7)
    usage (EXIT_FAILURE);
  else if (argc > 0)
    {
      int *dates[] = { &ft_chg.rel_year, &ft_chg.rel_month, &ft_chg.rel_day };
      intmax_t *rel_times[] =
        { &ft_chg.rel_hour, &ft_chg.rel_minutes, &ft_chg.rel_seconds };
      struct tm_ptrs ft_ptrs =
        (struct tm_ptrs) { .dates = dates, .rel_times = rel_times,
                           .ns = &ft_chg.rel_ns };
      char *errarg = NULL;

      set_num = argreltm (argc,  (const char **)argv, &ft_ptrs, &errarg);
      if (set_num < 0)
        error (EXIT_FAILURE, 0, "invalid relative time '%s'", errarg);
      else if (set_num == 0 || errarg != NULL)
        usage (EXIT_FAILURE);

      ft_chg.rel_set = true;
    }

  /* Get the current time on system clock or referenced time from a file. */
  if (!ref_file.name)
    {
      if (! currentft (now))
        error (EXIT_FAILURE, ERRNO (), "failed to get system clock");

      for (i = 0; i < FT_SIZE; i++)
        now[i] = now[0];
    }
  else if (! getft (now, &ref_file))
    errfile (EXIT_FAILURE, ERRNO (), "failed to get attributes of", &ref_file);

  if (ftind >= 0)
    {
      for (i = 0; i < FT_SIZE; i++)
        {
          if (i ^ ftind)
            ft_nowp[i] = NULL;
        }
    }
  else  /* Output the last write time by default. */
    ftind = FT_MTIME;

  /* Generate a new sequence at once before get random values. */
  if (IS_FT_NSEC_RANDOMIZING (ft_chg.modflag))
    srandsec (--seed);

  /* Change the file time for the file of the specified name by parameters
     gotten from arguments or output its value elapsed since a time. */
  if (ft_file.name)
    {
      int open_errno;

      if (! getft (ft, &ft_file)
          && (open_errno = ERRNO (), ! ERRFILE_NOT_FOUND (open_errno)))
        errfile (EXIT_FAILURE, open_errno,
                 "failed to get attributes of", &ft_file);

        OPEN_FILE (&ft_file, true);

        if (IS_INVALID_FILE (&ft_file, false))
          errfile (EXIT_FAILURE, ERRNO (), "failed to open", &ft_file);

      success = setft (&ft_file, ft_nowp, &ft_chg);
    }
  else  /* ft_file.name == NULL */
    {
      success = calcft (ft, now + ftind, &ft_chg);

      if (success)
        {
          if (seconds_output)  /* Seconds since 1970-01-01 00:00 UTC */
            {
              int frac_val;

              success = ft2sec (ft, &ft_elapse, &frac_val);

              if (success)
                ft_frac_val = frac_val;
            }
          else  /* 100 nanoseconds since 1601-01-01 00:00 UTC */
            success = ft2val (ft, 0, &ft_elapse);
        }

      printelapse (false, ft_elapse, ft_frac_val);
    }

# ifndef USE_TM_GLIBC
  if (wargv)
    LocalFree (wargv);
# endif

  return success ? EXIT_SUCCESS : EXIT_FAILURE;
}
#endif
