/* argmatch.h -- Matching an argument with the name in tables

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

/* The table of a argument name and value  */

struct arg_table
{
  const char *name;  /* Must be NULL if not followed by this argument */
  int value;
};

/* Compare the leading part of the specified argument with a name member
   in *TABLE case-insensitively and set its value member into *VALUE
   if equal to the whole or ABBRLEN characters, storing the pointer to
   a following character into *ENDPTR. Continue to compare with following
   tables unless the name member is not NULL. Return true if matched.  */

bool argmatch (const char *arg, const struct arg_table *table,
               int abbrlen, int *value, char **endptr);

/* Output the list of valid arguments for *TABLE to standard error.  */

void argmatch_valid (const struct arg_table *table);
