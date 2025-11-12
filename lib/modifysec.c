/* Modify seconds and nanoseconds since Unix epoch
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

#include "config.h"

#ifdef USE_TM_CYGWIN
# include <time.h>
#else
# include <windows.h>
#endif
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "ft.h"
#include "imaxoverflow.h"
#include "modifysec.h"
#include "timeoverflow.h"

/* Return the value of 100 nanoseconds less than a second in current time
   if successful, otherwise, FT_NSEC_PRECISION. */
static int
currentns ()
{
  FT ft;
  int ns = FT_NSEC_PRECISION;

  if (currentft (&ft))
    {
#ifdef USE_TM_CYGWIN
      ns = ft.tv_nsec / 100;
#else
      LARGE_INTEGER ft_large;

      ft_large.HighPart = ft.dwHighDateTime;
      ft_large.LowPart = ft.dwLowDateTime;

      ns = ft_large.QuadPart % FT_NSEC_PRECISION;
#endif
    }

  return ns;
}

/* Generate a new sequence of pseudo-random values for the specified seed
   number. If SEED is less than 0, use current time instead.  */

void
srandsec (int seed)
{
  if (seed < 0)
    seed = currentns ();

  srand (seed);
}

#define RAND_USE_MAX        32767
#define RAND_USE_DIGIT_SIZE 3
#if 0
# define RAND_REST_DIGIT_MAX 31
#endif
#define RAND_REST_BITS      5

/* Return the random value of 100 nanoseconds less than a second.  */
static int
randns ()
{
  int rand_ns = 0;
  int rand_rest = 0;
  int i;
  for (i = 0; i < 2; i++)
    {
      int r = rand () % (RAND_USE_MAX + 1);
      int j;
      for (j = 0; j < RAND_USE_DIGIT_SIZE; j++)
        {
          rand_ns += r % 10;
          rand_ns *= 10;
          r /= 10;
        }
      rand_rest <<= RAND_REST_BITS;
      rand_rest = r % (1 << RAND_REST_BITS);
    }

  return rand_ns + rand_rest % 10;
}

/* Return the value into which digits of the specified 100 nanoseconds less
   than a second are arranged at random.  */
static int
arrangens (int nsec, bool nsec_random)
{
  char ns_digits[FT_NSEC_DIGITS] = { 0 };
  int ns = 0;
  int arrange_key = nsec_random ? nsec : randns ();
  int i;

  /* Place digits of the specified nanoseconds into each position
     in the array, arranged by the remainder divided by the number
     from FT_NSEC_DIGITS to 1 If conflict, find an empty position
     by the increment. */
  for (i = FT_NSEC_DIGITS; i > 0; i--)
    {
      int ordinal = arrange_key % i;
      while (ns_digits[ordinal])
        ordinal = (ordinal + 1) % FT_NSEC_DIGITS;

      ns_digits[ordinal] = '0' + nsec % 10;
      nsec /= 10;
    }

  for (i = FT_NSEC_DIGITS - 1; i >= 0; i--)
     ns = ns * 10 + ns_digits[i] - '0';

  return ns;
}

/* Modify the specified value of seconds since 1970-01-01 00:00 UTC and 100
   nanoseconds less than a second, according to MODFLAG. Set its two values
   back into *SECONDS and *NSEC and return true if *NSEC is not less than 0
   and modification is performed, otherwise, return false.  */

bool
modifysec (intmax_t *seconds, int *nsec, int modflag)
{
  if (*nsec >= 0)
    {
      intmax_t sec = *seconds;
      int ns = *nsec;

      if (ns && SECONDS_ROUNDING (modflag))
        {
          /* Give priority to round seconds up if both are specified. */
          if (modflag & SECONDS_ROUND_UP
              && (IMAX_ADD_WRAPV (1, sec, &sec) || timew_overflow (sec)))
            return false;

          ns = 0;
        }

      if (modflag & NSEC_RANDOM)
        ns = randns ();

      if (modflag & NSEC_ARRANGE)
        ns = arrangens (ns, modflag & NSEC_RANDOM);

      *seconds = sec;
      *nsec = ns;

      return true;
    }

  return false;
}

#ifdef TEST
# include <stdio.h>
# include <unistd.h>

# include "cmdtmio.h"
# include "error.h"
# include "exit.h"

char *program_name = "modifysec";

static void
usage (int status)
{
  printusage ("modifysec", " [-]SECONDS[.nnnnnnn] NUMBER\n\
Repeat the modification of SECONDS since 1970-01-01 00:00 UTC NUMBER\n\
times. Display modified value if preformed, otherwise, -1.\n\
\n\
Options:\n\
  -A        arrange digits of nanoseconds at random\n\
  -C        round up to the smallest second that is not less than seconds\n\
  -F        round down to the largest second that does not exceed seconds\n\
  -R SEED   set nanoseconds at random by SEED; If 0, use current time",
true, 0);
  exit (status);
}

int
main (int argc, char **argv)
{
  intmax_t seconds;
  int nsec = 0;
  int modflag = 0;
  int repeat_num = 1;
  int seed = 0;
  int c, i;
  int status = EXIT_SUCCESS;
  int set_num;
  char *endptr;
  struct tm_ptrs tm_ptrs = (struct tm_ptrs) { .elapse = &seconds,
                                              .frac_val = &nsec };
  struct tm_fmt tm_fmt = { false };

  while ((c = getopt (argc, argv, ":ACFR:")) != -1)
    {
      switch (c)
        {
        case 'A':
          modflag |= NSEC_ARRANGE;
          break;
        case 'C':
          modflag |= SECONDS_ROUND_UP;
          break;
        case 'F':
          modflag |= SECONDS_ROUND_DOWN;
          break;
        case 'R':
          modflag |= NSEC_RANDOM;
          set_num = sscannumuint (optarg, &seed, &endptr);
          if (set_num < 0)
            error (EXIT_FAILURE, 0, "invalid seed value %s", optarg);
          else if (set_num == 0 || *endptr != '\0')
            usage (EXIT_FAILURE);
          break;
        default:
          usage (EXIT_FAILURE);
        }
    }

  argc -= optind;
  argv += optind;

  if (SECONDS_ROUNDING (modflag) == (SECONDS_ROUND_DOWN | SECONDS_ROUND_UP))
    error (EXIT_FAILURE, 0, "cannot specify the both of rounding down and up");
  else if (argc < 1 || argc > 2)
    usage (EXIT_FAILURE);

  /* Set the first argument into seconds and its fractional part. */
  set_num = sscantm (*argv, &tm_ptrs, &endptr);
  if (set_num < 0)
    error (EXIT_FAILURE, 0, "invalid seconds %s", *argv);
  else if (set_num == 0 || *endptr != '\0')
    usage (EXIT_FAILURE);

  /* Set the next argument of seconds into the repeat number if specified. */
  if (--argc > 0)
    {
      set_num = sscannumuint (*++argv, &repeat_num, &endptr);
      if (set_num < 0)
        error (EXIT_FAILURE, 0, "invalid repeat number %s", *argv);
      else if (set_num == 0 || *endptr != '\0')
        usage (EXIT_FAILURE);
    }

  /* Generate a new sequence at once before get random values. */
  if (modflag & (NSEC_RANDOM | NSEC_ARRANGE))
    srandsec (--seed);

  for (i = 0; i < repeat_num; i++)
    {
      intmax_t sec = seconds;
      int ns = nsec;

      tm_ptrs.elapse = &sec;
      tm_ptrs.frac_val = &ns;

      if (! modifysec (&sec, &ns, modflag))
        {
          sec = -1;
          tm_ptrs.frac_val = NULL;

          status = EXIT_FAILURE;
        }

      printtm (&tm_fmt, &tm_ptrs);
    }

  return status;
}
#endif
