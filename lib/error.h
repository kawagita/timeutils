/* Declaration for error-reporting function
   Copyright (C) 1995, 1996, 1997, 2003, 2006 Free Software Foundation, Inc.
   Copyright (C) 2025 Yoshinori Kawagita.
   This file is part of the GNU C Library.

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

#ifndef _ERROR_H
#define _ERROR_H 1

/* Return the number of an error that occured the most recently  */
#ifdef USE_TM_CYGWIN
# include <errno.h>

# define ERRNO() errno
#else
# define ERRNO() GetLastError ()
#endif

/* Print a message with `fprintf (stderr, FORMAT, ...)';
   if ERRNUM is nonzero, follow it with ": " and strerror (ERRNUM).
   If STATUS is nonzero, terminate the program with `exit (STATUS)'.  */

extern void error (int __status, int __errnum, const char *__format, ...);

#endif /* error.h */
