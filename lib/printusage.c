/* Output the usage of a program to standard output
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

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>

/* Output the usage of the specified program name and description
   to standard output. If HAS_OPTIONS is true, output "[OPTION]..."
   in the back of *NAME, and if TRANS_NO_DST_OPTION is an alphabet,
   output the description of adjusting by DST offset for a time that
   is skipped over and repeated in its transition date.  */

void
printusage (const char *name, const char *desc,
            bool has_options, int trans_no_dst_option)
{
  char *optdesc = has_options ? " [OPTION]..." : "";

#ifdef USE_TM_WRAPPER
  if (isalpha (trans_no_dst_option))
    printf ("Usage: %s%s [-%c]%s\n\
\n\
With -%c, don't adjust time by DST offset for a time that is skipped over\n\
and repeated in its transition date.\n\
", name, optdesc, trans_no_dst_option, desc, trans_no_dst_option);
  else
#endif
    printf ("Usage: %s%s%s\n", name, optdesc, desc);
}
