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
#include <stdint.h>
#include <stdio.h>

#include "cmdtmio.h"

/* Output the usage of the specified program name and description to
   standard output. If HAS_OPTIONS is true, output "[OPTION]..." in
   the back of *NAME, and if HAS_ISDST is true or TRANS_NO_DST_OPTION
   is an alphabet option, output the description of adjusting by DST
   offset in the ordinary or transition date.  */

void
printusage (const char *name, const char *desc,
            bool has_options, bool has_isdst, int trans_no_dst_option)
{
  printf ("Usage: %s", name);

  if (has_options)
    fputs (" [OPTION]...", stdout);

#ifdef USE_TM_SELFIMPL
  bool has_no_dst_option = isalpha (trans_no_dst_option);
  if (has_no_dst_option)
    printf (" [-%c]", trans_no_dst_option);
#endif

  printf ("%s\n", desc);

#ifdef USE_TM_SELFIMPL
  if (has_no_dst_option)
    {
      if (has_isdst)
        {
          printf ("\
\n\
If \"" DST_NAME "\" or \"" ST_NAME "\" is specified, adjust time by\
 DST offset of current\n\
time zone or not. With -%c, don't adjust for a time that is skipped\n\
over and repeated in its transition date.\n\
", trans_no_dst_option);
        }
      else
        printf ("\
\n\
With -%c, don't adjust time by DST offset for a time that is skipped\n\
over and repeated in its transition date.\n\
", trans_no_dst_option);
    }
  else
#endif
  if (has_isdst)
    printf ("\
\n\
If \"" DST_NAME "\" or \"" ST_NAME "\" is specified, adjust time by\
 DST offset of current\ntime zone or not.\n\
");
}
