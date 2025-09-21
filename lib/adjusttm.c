/* Adjust parameters of time to the range of correct values
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
   awith this program; if not, write to the Free Software Foundation,
   Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.  */

#include <stdbool.h>

#include "intoverflow.h"

/* Decrease or increase PARAM2 toward zero by the multiple of BASE as many as
   possible and increase or decrease PARAM1 by its multiple of UNIT if PARAM2
   is positive or negative. Return true if not overflow or divided by zero,
   otherwise, false.  */

bool
adjusttm (int *param1, int unit, int *param2, int base)
{
  if (base)
    {
      int delta = *param2 / base;

      if (delta)
        {
          if (INT_ADD_WRAPV (*param1, delta * unit, param1))
            return false;

          *param2 -= delta * base;
        }

      return true;
    }

  return false;
}

/* Bring LOWPARAM into the range of 0 to BASE - 1 and increase or decrease
   HIGHPARAM by its carried value if LOWPARAM is positive or negative. Return
   true if not overflow or divided by zero, otherwise, false.  */

bool
carrytm (int *highparam, int *lowparam, int base)
{
  if (base)
    {
      int delta = *lowparam >= 0
                  ? *lowparam / base : (*lowparam + 1) / base - 1;

      if (delta)
        {
          if (INT_ADD_WRAPV (*highparam, delta, highparam))
            return false;

          *lowparam -= delta * base;
        }

      return true;
    }

  return false;
}
