/* Compare an argument with the name in tables
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
#include <string.h>

#include "argmatch.h"

/* Compare the leading part of the specified argument with a name member
   in *TABLE case-insensitively and set its value member into *VALUE
   if equal to the whole or ABBRLEN characters, storing the pointer to
   a following character into *ENDPTR. Continue to compare with following
   tables unless the name member is not NULL. Return true if matched.  */

bool
argmatch (const char *arg, const struct arg_table *table,
          int abbrlen, int *value, char **endptr)
{
  const char *p = arg;

  *endptr = (char *)p;

  if (! isspace (*p) && ! ispunct (*p) && *p != '\0')
    {
      while (table->name)
        {
          int name_len = strlen (table->name);
          if (name_len > 0)
            {
              int len = 0;

              /* Input characters from the specified argument to a space
                 or punctuation and compare it with the word in tables. */
              do
                {
                  if (toupper (*p) != toupper (table->name[len]))
                    break;
                  p++;
                }
              while (++len < name_len
                     && ! isspace (*p) && ! ispunct (*p) && *p != '\0');

             if ((len == name_len || len == abbrlen)
                 && (isspace (*p) || ispunct (*p) || *p == '\0'))
               {
                 *endptr = (char *)p;
                 *value = table->value;

                 return true;
               }
            }

          table++;
        }
    }

  return false;
}

#define _(msgid) msgid

/* Output the list of valid arguments for *TABLE to standard error.  */

void
argmatch_valid (const struct arg_table *table)
{
  fputs (_("Valid arguments are:"), stderr);

  if (table->name)
    {
      int value = table->value < 0 ? 0 : -1;
      do
        {
          if (value != table->value)
            {
              /* Start to output new line of arguments for this value. */
              fprintf (stderr, _("\n  - '%s'"), table->name);
              value = table->value;
            }
          else
            fprintf (stderr, ", '%s'", table->name);

          table++;
        }
      while (table->name);
    }
  else
    fputs (_(" Nothing."), stderr);

  fputc ('\n', stderr);
}
