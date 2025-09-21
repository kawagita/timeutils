/* config -- Configuration of this package

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

/* The name and version of this package  */
#define PACKAGE_NAME    "timeutils"
#define PACKAGE_VERSION "1.0"

#ifndef USE_TM_CYGWIN
/* Set the macro to not define _USE_32BIT_TIME_T in _mingw.h  */
# define __MINGW_USE_VC2005_COMPAT
#endif
