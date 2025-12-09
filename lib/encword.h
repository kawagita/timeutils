/* encword.h -- Getting an encoding word from the string

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

/* Get a word encoding by the specified codepage from the leading part in
   the specified string, and set those characters into the array pointed by
   WORD but no more than MAXSIZE bytes are not placed. Return the number
   of word's bytes, or -1 if MAXSIZE is zero.  */

size_t encword (char *word, size_t maxsize, const char *str, int ansi_cp);
