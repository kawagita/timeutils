/* Error handler for noninteractive utilities
   Copyright (C) 1990-1998, 2000-2005, 2006 Free Software Foundation, Inc.
   Copyright (C) 2025 Yoshinori Kawagita.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along
   with this program; if not, write to the Free Software Foundation,
   Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.  */

/* Written by David MacKenzie <djm@gnu.ai.mit.edu>.  */

#include "error.h"

#ifndef USE_TM_CYGWIN
# include <windows.h>
#endif
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef _
# define _(String) String
#endif

#ifdef FREE_WARGV
extern LPWSTR *wargv;
#endif

/* The calling program should define program_name and set it to the
   name of the executing program.  */
extern char *program_name;

/* Print the system error message corresponding to ERRNUM.  */
void
print_errno_message (int errnum)
{
  char const *s;
  char errbuf[1024];

#ifdef USE_TM_CYGWIN

  if (strerror_r (errnum, errbuf, sizeof errbuf) == 0)
#else
  if (FormatMessage (FORMAT_MESSAGE_FROM_SYSTEM |
                     FORMAT_MESSAGE_IGNORE_INSERTS,
                     NULL, (DWORD)errnum,
                     MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT),
                     (LPSTR)&errbuf, sizeof errbuf, NULL))
#endif
    s = errbuf;
  else
    s = _("Unknown system error");

  fprintf (stderr, ": %s", s);
}

/* Print the program name and error message MESSAGE, which is a printf-style
   format string with optional args.
   If ERRNUM is nonzero, print its corresponding system error message.
   Exit with status STATUS if it is nonzero.  */
void
error (int status, int errnum, const char *message, ...)
{
  va_list args;

  fflush (stdout);
  fprintf (stderr, "%s: ", program_name);

  va_start (args, message);
  vfprintf (stderr, message, args);
  va_end (args);

  if (errnum)
    print_errno_message (errnum);
  fputc ('\n', stderr);

  fflush (stderr);
  if (status)
    {
#ifdef FREE
      if (wargv)
        LocalFree (wargv);
#endif
      exit (status);
    }
}
