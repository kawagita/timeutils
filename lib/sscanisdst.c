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

#include "cmdtmio.h"

/* Parse the leading part of the specified argument as the setting name
   whether the time is adjusted by DST offset and set its value into
   *ISDST, storing the pointer to a following character into *ENDPTR.
   Return 1 or 0 if a value is set or not.  */

int
sscanisdst (const char *argv, int *isdst, char **endptr)
{
  const struct word_table isdst_table[] =
    { { DST_NAME, 1 }, { ST_NAME, 0 }, { NULL, -1 } };
  int isdst_val;
  int set_num = sscanword (argv, isdst_table, 0, &isdst_val, endptr);
  if (set_num > 0)
    *isdst = isdst_val;

  return set_num;
}
