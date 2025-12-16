/* imaxcheck.h -- Check of intmax_t overflow

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

/* Macro definitions for overflow function  */
#define IMAX_ADD_WRAPV(a,b,p)      imax_add_overflow (a, b, p)
#define IMAX_SUBTRACT_WRAPV(a,b,p) imax_sub_overflow (a, b, p)
#define IMAX_MULTIPLY_WRAPV(a,b,p) imax_mul_overflow (a, b, p)

/* Check whether the result value overflows from the range of intmax_t
   by the addition of the specified A and B. Return false and set its
   value into *P if don't overflow, otherwise, return true.  */

bool imax_add_overflow (intmax_t a, intmax_t b, intmax_t *p);

/* Check whether the result value overflows from the range of intmax_t
   by the subtraction of the specified A and B. Return false and set its
   value into *P if don't overflow, otherwise, return true.  */

bool imax_sub_overflow (intmax_t a, intmax_t b, intmax_t *p);

/* Check whether the result value overflows from the range of intmax_t
   by the multiplication of the specified A and B. Return false and set
   its value into *P if don't overflow, otherwise, return true.  */

bool imax_mul_overflow (intmax_t a, intmax_t b, intmax_t *p);
