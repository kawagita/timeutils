/* Check whether integer value overflows by the calculation
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

#include <limits.h>
#include <stdbool.h>

/* Check whether the result value overflows from the range of integer
   by the addition of the specified A and B. Return false and set its
   value into *P if don't overflow, otherwise, return true.  */

bool
int_add_overflow (int a, int b, int *p)
{
  if (b > 0 ? a > INT_MAX - b : a < INT_MIN - b)
    return true;
  *p = a + b;
  return false;
}

/* Check whether the result value overflows from the range of integer
   by the subtraction of the specified A and B. Return false and set its
   value into *P if don't overflow, otherwise, return true.  */

bool
int_sub_overflow (int a, int b, int *p)
{
  if (b < 0 ? a > INT_MAX + b : a < INT_MIN + b)
    return true;
  *p = a - b;
  return false;
}

/* Check whether the result value overflows from the range of integer
   by the multiplication of the specified A and B. Return false and set
   its value into *P if don't overflow, otherwise, return true.  */

bool
int_mul_overflow (int a, int b, int *p)
{
  if (b > 0
     ? (a > INT_MAX / b) || (a < INT_MIN / b)
     : (b < -1
        ? (a > INT_MIN / b) || (a < INT_MAX / b)
        : b == -1 && a == INT_MIN))
    return true;
  *p = a * b;
  return false;
}
