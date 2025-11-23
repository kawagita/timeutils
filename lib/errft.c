/* Output the error message of file time in NTFS
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

#ifdef USE_TM_GLIBC
# include <time.h>
#else
# ifndef UNICODE
#  define UNICODE
# endif
# include <windows.h>
#endif

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "ft.h"
#include "errft.h"
#include "error.h"

#ifndef USE_TM_GLIBC
extern LPWSTR *wargv;

extern void print_errno_message (int errnum);
#endif

/* The calling program should define program_name and set it to the
   name of the executing program.  */
extern char *program_name;

/* Print a message for the specified file, leading characters in the array
   pointed to DESC. Use STATUS or ERRNUM same as the error function.  */

void
errfile (int status, int errnum, const char *desc, struct file *file)
{
#ifdef USE_TM_GLIBC
  error (status, errnum, "%s%s", desc, file->name);
#else
  HANDLE hStderr = GetStdHandle(STD_ERROR_HANDLE);
  DWORD len;

  fprintf (stderr, "%s: ", program_name);
  fputs (desc, stderr);

  WriteConsole (hStderr, file->name, wcslen (file->name), &len, NULL);

  if (errnum)
    print_errno_message (errnum);

  if (status)
    {
      if (wargv)
        LocalFree (wargv);

      exit (status);
    }
#endif
}
