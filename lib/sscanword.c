/* Compare the argument with the word in tables
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
#include <string.h>

#include "cmdtmio.h"

/* Compare the leading part of the specified argument with a name member
   in *TABLE case-insensitively and set its value member into *VALUE
   if equal to the whole or ABBRLEN characters, storing the pointer to
   a following character into the *ENDPTR. Continue to compare with
   following tables unless the name member is not NULL. Return 1 or 0
   if a value is set or not.  */

int
sscanword (const char *argv, const struct word_table *table,
           int abbrlen, int *value, char **endptr)
{
  while (isspace (*argv))
    argv++;

  *endptr = (char *)argv;

  if (! ispunct (*argv) && *argv != '\0')
    {
      while (table->name)
        {
          const char *name = table->name;
          int name_len = strlen (name);
          if (name_len > 0)
            {
              const char *p = argv;
              int len = 0;

              /* Input characters from the specified argument to a space
                 or punctuation and compare it with the word in tables. */
              do
                {
                  if (toupper (*p) != toupper (name[len]))
                    break;
                  p++;
                }
              while (++len < name_len
                     && ! isspace (*p) && ! ispunct (*p) && *p != '\0');

             if (len == name_len || len == abbrlen)
               {
                 bool spaced = isspace (*p);
                 if (spaced || ispunct (*p) || *p == '\0')
                   {
                     if (spaced)
                       while (isspace (*++p));

                     *endptr = (char *)p;
                     *value = table->value;

                     return 1;
                   }
               }
            }

          table++;
        }
    }

  return 0;
}
