/* Get a word encoding by ANSI codepage from the string
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
#include <stdlib.h>

/* Return true if a character is the first byte of Shift_JIS encoding.  */
#define SJIS_FIRST(c) ((c >= 0x81 && c <= 0x9f) || (c >= 0xe0 && c <= 0xfc))

/* Return true if a character is the first byte of Big5 encoding.  */
#define BIG5_FIRST(c) (c >= 0x81 && c <= 0xfe)

/* Get a word encoding by the specified codepage from the leading part in
   the specified string, and set those characters into the array pointed by
   WORD but no more than MAXSIZE bytes are not placed. Return the number
   of word's bytes, or -1 if MAXSIZE is zero.  */

size_t
encword (char *word, size_t maxsize, const char *str, int ansi_cp)
{
  const char *input = str;
  unsigned char c = *str;
  char *p = word;

  if (maxsize == 0)
    return -1;

  bool sjis_or_big5 = ansi_cp == 932 || ansi_cp == 950;
  bool last_char_adjusted = sjis_or_big5;
  bool first_byte_input = false;
  do
    {
      bool double_byte_char = sjis_or_big5;

      if (c == '\0')
        {
          /* Place '\0' into the previous value input as the first byte
             of a double byte character if no second byte is input. */
          if (first_byte_input)
            *(p - 1) = '\0';
          break;
        }
      else if (! first_byte_input)
        {
          if (! isalnum (c))
            {
              if (c <= 0x7f)  /* No alphnumeric character of ASCII */
                break;

              switch (ansi_cp)
                {
                case 932:  /* Codepage of Shift_JIS */
                  double_byte_char = first_byte_input = SJIS_FIRST (c);
                  break;
                case 950:  /* Codepage of Big5 */
                  double_byte_char = first_byte_input = BIG5_FIRST (c);
                  break;
                }
            }
        }
      else if (sjis_or_big5)
        first_byte_input = false;

      if (p < word + maxsize - 1)
        *p++ = c;
      else if (last_char_adjusted)
        {
          /* Place '\0' into the previous value input as the first byte
             of a double byte character if no second byte is placed. */
          if (double_byte_char && ! first_byte_input)
            *(p - 1) = '\0';
          last_char_adjusted = false;
        }
      c = *++input;
    }
  while (true);

  *p = '\0';

  return input - str;
}
