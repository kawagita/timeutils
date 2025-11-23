/* ft.h -- File time in NTFS, represented by 100 nanoseconds unit since
           1601-01-01 00:00 UTC on Windows or struct timespec on Cygwin

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

/* The structure into which file time is stored  */

#ifdef USE_TM_GLIBC
typedef struct timespec FT;
#else
typedef FILETIME FT;
#endif

/* The maximum and minimum value of seconds since 1601-01-01 00:00 UTC
   in file time  */

#define FT_SECONDS_MAX  922337203685
#define FT_SECONDS_MIN -922337203685

/* The precision and number of digits for a nanosecond in file time  */

#define FT_NSEC_PRECISION 10000000
#define FT_NSEC_DIGITS    7

/* 100 nanoseconds to 1970-01-01 00:00 UTC in file time  */

#define FT_UNIXEPOCH_NSEC 116444736000000000

/* Get the current time on system clock as file time in NTFS. Set its value
   into *FT and return true if successfull, otherwise, return false.  */

bool currentft (FT *ft);

/* Return the value of 100 nanoseconds since 1601-01-01 00:00 UTC converted
   from the specified file time, according to SEC_MODFLAG. If conversion is
   not performed, return 0.  */

intmax_t toftval (const FT *ft, int sec_modflag);

/* Convert the specified file time to seconds since 1970-01-01 00:00 UTC
   and 100 nanoseconds less than a second. Set its two values into *SECONDS
   and *NSEC and return true if conversion is performed, otherwise, return
   false.  */

bool ft2sec (const FT *ft, intmax_t *seconds, int *nsec);

/* Convert the specified seconds since 1970-01-01 00:00 UTC and 100
   nanoseconds less than a second to file time. Set its value into *FT
   and return true if NSEC is not less than 0 and conversion is performed,
   otherwise, return false.  */

bool sec2ft (intmax_t seconds, int nsec, FT *ft);

/* The size or each index of file times, set in a file  */

#ifdef USE_TM_GLIBC
# define FT_SIZE 2
#else
# define FT_SIZE 3
#endif

#define FT_ATIME 0  /* Last access time */
#define FT_MTIME 1  /* Last write time */
#define FT_CTIME 2  /* Creation time */

/* The structure of a file  */

struct file
{
#ifdef USE_TM_GLIBC
  const char *name;
  int fd;
  bool no_dereference;
#else
  LPCWSTR name;
  HANDLE hFile;
#endif
  bool isdir;
};

/* Get file times for the specified struct file into FT and set the flag
   of a directory into the isdir member in *FT_FILE. If the no_dereference
   member is true, get the time of symbolic link on Cygwin but not a file
   referenced by it. Return true if successfull, otherwise, false.  */

bool getft (FT ft[FT_SIZE], struct file *ft_file);

/* The change to file time  */

typedef struct
{
  /* If the datetime_unset member is true, any parameter of date and time
     in this structure is not set into file time  */
  bool datetime_unset;

  /* The date and time adapted to file time before the modification  */
  bool date_set;
  int year;
  int month;
  int day;
  int hour;
  int minutes;
  int seconds;
  int ns;

  int sec_modflag;

  /* Positive or negative values to modify file time   */
  bool rel_set;
  int rel_year;
  int rel_month;
  int rel_day;
  intmax_t rel_hour;
  intmax_t rel_minutes;
  intmax_t rel_seconds;
  int rel_ns;

  /* The value by which file time is changed to a week day  */
  int day_number;
  intmax_t day_ordinal;

  /* The offset of a time zone in seconds for file time  */
  bool tz_set;
  int tz_offset;

  /* The value set into the isdst member in struct tm given to mktime
     function if file time is changed for local time zone  */
  int lctz_isdst;
} FT_CHANGE;

/* Calculate file time from *NOW by members in *TM_CHG and set its value
   into *FT. If the datetime_unset member is true, not calculate for *NOW
   and copy it to *FT directly. Return true if not overflow, otherwise,
   false.  */

bool calcft (FT *ft, const FT *now, const FT_CHANGE *ft_chg);

/* Change the specified file time by members in *TM_CHG and set its value
   to the file specified by *FT_FILE. If an address in the array pointed
   to FT_NOWP is NULL, its corresponding file time is not changed. Return
   true if successfull, otherwise, false.  */

bool setft (struct file *ft_file, const FT *ft_nowp[FT_SIZE],
            const FT_CHANGE *ft_chg);

/* Parameters parsed from a date and time string, setting file time  */

typedef union
{
  /* Parameters of Unix time, specified by "@ SECONDS[.nnnnnnn]"  */
  struct
    {
      bool seconds_set;
      intmax_t seconds;
      int nsec;
    } timespec;

  /* Parameters by which file time is changed  */
  FT_CHANGE ft_change;
} FT_PARSING;

/* Parse the specified string as parameters of setting file time and set
   those values into *FT_PARSING. Return true if its string is correct,
   otherwise, false.  */

bool parseft (FT_PARSING *ft_parsing, const char *str);
