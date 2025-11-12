/* Get the number of leap days between two years
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

#include "adjusttm.h"
#include "intoverflow.h"

#ifdef TEST
# include <stdio.h>

/* The flag whether the table of leap days is output  */
static bool table_output = false;

/* The format to output the number of a year or leap days, and spaces  */
static char year_format[10] = "  %4d  |";
static char spaces_format[10] = "  %4s  |";
static char ldays_format[11] = "  %4d day";

/* The length of lines separating a year, duration, and leap days  */
static int line_lengths[3] = { 6, 6, 11 };

/* Output the line of separating a table.   */
static void
printline ()
{
  int i;
  for (i = 0; i < 3; i++)
    {
      int len = line_lengths[i];
      fputc (' ', stdout);
      while (--len >= 0)
        fputc ('-', stdout);
      if (i < 2)
        fputs (" |", stdout);
    }
  fputc ('\n', stdout);
}

/* Return the sign of '+' or '-' if the specified year is divisible by 400
   or 100, otherwise, return ' '  */
# define YEAR_DIV100_MARK(year) (year % 100 ? ' ' : (year % 400 ? '-' : '+'))

/* Output the number of the specified year in a table.   */
static void
printyear (int year, int ydiv100_mark)
{
  printf (year_format, year);
  printf (spaces_format, "");
  if (ydiv100_mark != ' ')
    printf (" %c\n", ydiv100_mark);
  else
    fputc ('\n', stdout);
}

/* Output a table of leap days from the secified start to end year.  */
static void
printtable (int ystart, int yend, int ldays)
{
  int yterm = yend - ystart;
  if (yterm >= 0)
    {
      printline ();

      /* Output the number of start year, duration and leap days, and end
         year in three lines if two years are different but further output
         years divisible by 100 under 400 years, othewise, in a line. */
      if (yterm)
        {
          if (yterm < 399)
            {
              if (yend % 100 != 0)
                printyear (yend, ' ');

              /* Output year numbers divisible by 100 in each line if those
                 are included from start to end year. */
              yend -= yend % 100 + (yend < 0 && yend % 100 != 0 ? 100 : 0);
              for (; ystart < yend; yend -= 100)
                {
                  int ydiv100_mark = YEAR_DIV100_MARK (yend);
                  if (ydiv100_mark == '-')
                    ldays--;

                  printyear (yend, ydiv100_mark);
                }
            }
          else
            printyear (yend, ' ');

          printf (spaces_format, "");
        }
      else
        {
          printf (year_format, yend);
          ldays_format[1] = YEAR_DIV100_MARK (yend);
        }

      int ydiv100_mark = ' ';
      if (yterm < 399)
        {
          ydiv100_mark = YEAR_DIV100_MARK (ystart);
          if (ydiv100_mark == '-')
            ldays--;
        }

      printf (year_format, yterm + 1);
      printf (ldays_format, ldays);
      if (ldays > 1)
        fputc ('s', stdout);
      fputc ('\n', stdout);
      ldays_format[1] = ' ';

      if (yterm)
        printyear (ystart, ydiv100_mark);
    }
}
#endif

/* Return true if a year divisible by DIV is included in TERM since YEAR  */
#define INCLUDE_DIV_YEAR(year,term,div) \
  (((year % div + (div - 1)) % div + (term)) >= div)

/* Calculate the number of leap days included in a duration between
   the specified two years. If TO_YEAR is more or less than FROM_YEAR,
   return the positive or negateive value, otherwise, if a leap day is
   included in TO_YEAR same as FROM_YEAR or not, return 1 or 0.  */

int
leapdays (int from_year, int to_year)
{
  int ldays = 0;
  int ystart = from_year;
  int yend = to_year;
  int yterm = 0;
  int delta = 0;

  if (from_year > to_year)
    {
      ystart = to_year;
      yend = from_year;
    }

#ifdef TEST
  int yend0 = yend;
#endif

  /* Calculate the duration bertween two years including themselves. */

  if (INT_SUBTRACT_WRAPV (yend, ystart, &yterm) || yterm)
    {
      /* Count the number of 400 years into DELTA from start to end year
         and calculate the multiple of 97 leap days in its duration. */
      if (yterm)
        {
          INT_ADD_WRAPV (yterm, 1, &yterm);  /* Ignore the overflow */
          delta = yterm / 400;
        }
      else
        delta = yend / 400 - ystart / 400
                + ((yend % 400 - ystart % 400) / 400 ? 1 : 0);

      if (delta)
        {
          int yend_u400 = yend - delta * 400;
          ldays = delta * 97;

#ifdef TEST
          if (table_output)
            printtable (yend_u400 + 1, yend, ldays);
#endif

          yterm = yend_u400 - ystart + 1;
          yend = yend_u400;
        }

      /* Calculate the number of years divisible by 100 in the rest under
         400 years from end year, not including a year divisible by 400. */
      if (yterm)
        {
          int yend_next_mod400;  /* Minus value to back a duration */
          if (yend < INT_MAX)
            {
              int yend_next = yend + 1;
              yend_next_mod400 = yend_next % 400;
              if (yend_next > 0 && yend_next_mod400 != 0)
                yend_next_mod400 -= 400;
            }
          else
            yend_next_mod400 = INT_MIN % 400;

          int ydiv100s = (yend_next_mod400 % 100 - yterm) / -100;
          if (ydiv100s)
            {
              ldays -= ydiv100s;

              /* See below whether a year divisible by 400 is included
                 in the rest under 400 years.

                  yend_next_mod400  |  Year -400 is included ?
                 ------------------ | --------------------------
                        0 to  -99   |  Never included
                     -100 to -199   |  Included if ydiv100s > 2
                     -200 to -299   |  Included if ydiv100s > 1
                     -300 to -399   |  Included if ydiv100s > 0   */

              if (ydiv100s > 3 - (yend_next_mod400 / -100))
                ldays++;
            }

          /* Count the number of years divisible by 4 under 400 years. */
          delta = yterm / 4;
          if (delta)
            {
              int yend_u4 = yend - delta * 4;
              ldays += delta;

#ifdef TEST
              if (table_output)
                printtable (yend_u4 + 1, yend, delta);
#endif

              yterm = yend_u4 - ystart + 1;
              yend = yend_u4;
            }
        }

      /* Check whether a leap day is included in the rest under 4 years. */
      delta = yterm && INCLUDE_DIV_YEAR (ystart, yterm, 4) ? 1 : 0;
    }
  else if (! HAS_NOLEAPDAY (from_year))
    delta = 1;

  ldays += delta;

#ifdef TEST
  if (table_output)
    {
      printtable (ystart, yend, delta);
      printline ();
      printf (spaces_format, "");
      printf (year_format, yend0 - ystart + 1);
      printf (ldays_format, ldays);
      if (ldays > 1)
        fputc ('s', stdout);
      fputc ('\n', stdout);
    }
#endif

  return from_year <= to_year ? ldays : - ldays;
}

#ifdef TEST
# include <stdint.h>
# include <stdio.h>
# include <unistd.h>

# include "cmdtmio.h"
# include "error.h"
# include "exit.h"

char *program_name = "leapdays";

static void
usage (int status)
{
  printusage ("leapdays", " YEAR1 YEAR2\n\
Display the number of leap days between YEAR1 and YEAR2. If YEAR1 is\n\
not more than YEAR2, calculate its value by the increment from 1 Jan\n\
YEAR1 to 31 Dec YEAR2, otherwise, by the decrement.\n\
\n\
Options:\n\
  -t   output the table of leap days instead of a number\
", true, 0);
  exit (status);
}

static int
yearwidth (int year)
{
  int width = 0;
  if (year < 0)
    width = 1;
  do
    {
      year /= 10;
      width++;
    }
  while (year);
  return width;
}

int
main (int argc, char **argv)
{
  int years[2];
  int ldays;
  int c;

  while ((c = getopt (argc, argv, ":t")) != -1)
    {
      switch (c)
        {
        case 't':
          table_output = true;
          break;
        default:
          usage (EXIT_FAILURE);
        }
    }

  argc -= optind;
  argv += optind;

  if (argc < 1 || argc > 2)
    usage (EXIT_FAILURE);

  int i;
  for (i = 0; i < argc; i++)
    {
      char *endptr;
      int set_num = sscannumint (*argv, years + i, &endptr);
      if (set_num < 0)
        error (EXIT_FAILURE, 0, "invalid year %s", endptr);
      else if (set_num == 0 || *endptr != '\0')
        usage (EXIT_FAILURE);
      argv++;
    }

  if (argc == 1)
    years[1] = years[0];

  if (table_output)
    {
      char ldays_spaces_format[7] = "  %9s";
      int ystart = years[0] <= years[1] ? years[0] : years[1];
      int yend = years[0] <= years[1] ? years[1] : years[0];
      int yterm = INT_MAX;

      if (! INT_SUBTRACT_WRAPV (yend, ystart, &yterm))
        INT_ADD_WRAPV (yterm, 1, &yterm);

      int ystart_width = yearwidth (ystart);
      int yend_width = yearwidth (yend);
      int yterm_width = yearwidth (yterm);
      int ywidth = ystart_width < yend_width
                   ? (yterm_width < yend_width ? yend_width : yterm_width)
                   : (yterm_width < ystart_width ? ystart_width : yterm_width);
      if (ywidth > 4)
        {
          /* Set the output width of a year or leap days in the format to
             the greater width of the specified two years or its duration. */
          sprintf (year_format, "  %%%dd  |", ywidth);
          sprintf (spaces_format, "  %%%ds  |", ywidth);
          sprintf (ldays_format, "  %%%dd day", ywidth);
          sprintf (ldays_spaces_format, "  %%%ds", ywidth + 5);

          line_lengths[0] = line_lengths[1] = ywidth + 2;
          line_lengths[2] = ywidth + 7;
        }

      /* Output the header of numbers in the table. */
      printf (spaces_format, "Year");
      printf (spaces_format, "Term");
      printf (ldays_spaces_format, "Leap Days");
      fputc ('\n', stdout);
    }

  ldays = leapdays (years[0], years[1]);

  if (!table_output)
    printf ("%d\n" , ldays);

  return EXIT_SUCCESS;
}
#endif
