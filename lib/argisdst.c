/* Input the setting name of adjusting by DST offset from an argument
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

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "argempty.h"
#include "argmatch.h"
#include "cmdtmio.h"

/* Parse the leading part of the specified argument as the setting name
   whether the time is adjusted by DST offset in a time zone and set its
   value into *ISDST. Return true if "DST" or "ST".  */

bool
argisdst (const char *arg, int *isdst)
{
  const struct arg_table dst_settings[] =
    { { DST_NAME, 1 }, { ST_NAME, 0 }, { NULL, -1 } };
  char *endp;
  int isdst_val;
  if (argmatch (arg, dst_settings, 0, &isdst_val, &endp) && argempty (endp))
    {
      *isdst = isdst_val;
      return true;
    }

  return false;
}
