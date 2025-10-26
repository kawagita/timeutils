/* ft.h -- Definitions of file time in NTFS

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

/* The structure into which file time is set  */

#ifdef USE_TM_CYGWIN
typedef struct timespec FT;
#else
typedef FILETIME FT;
#endif

/* The precision and number of digits for a fractional second in file time  */

#define FT_FRAC_PRECISION 10000000
#define FT_FRAC_DIGITS    7

/* Seconds to Unix epoch in file time  */

#define FT_UNIXEPOCH_SECONDS 11644473600LL

/* Return the value of 100-nanoseconds unit elapsed since 1601-01-01 00:00
   UTC in FILETIME, converted from the specified file time. If conversion
   is not performed, return 0.  */

intmax_t toftval (const FT *ft);

/* Convert the specified file time to seconds and 100 nanoseconds since Unix
   epoch. Set its value into *SECONDS but nanoseconds into *NSEC unless not
   NULL. Return true if conversion is performed, otherwise, false.  */

bool ft2secns (const FT *ft, intmax_t *seconds, int *nsec);

/* Convert the specified seconds and 100 nanoseconds since Unix epoch to
   file time and set its value into *FT. Return true if NSEC is more than
   or equal to 0 and conversion is performed, otherwise, false.  */

bool secns2ft (intmax_t seconds, int nsec, FT *ft);

/* The size or each index of file times, set in a file  */

#ifdef USE_TM_CYGWIN
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
#ifdef USE_TM_CYGWIN
  const char *name;
#else
  LPCWSTR name;
#endif
  bool no_dereference;
  bool isdir;
};

/* The format to output the name of a file  */

#ifdef USE_TM_CYGWIN
# define PRIsNAME "s"
#else
# define PRIsNAME "ls"
#endif

/* Get file times for the specified struct file into FT and set the flag
   of a directory into the isdir member in *FT_FILE. If the no_dereference
   member is true, get the time of symbolic link on Cygwin but not a file
   referenced by it. Return true if successfull, otherwise, false.  */

bool getft (FT ft[FT_SIZE], struct file *ft_file);
