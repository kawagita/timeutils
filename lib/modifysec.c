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

/* This bit complements the random value from 30240 to 32767 because its
   value can only permute half digits as the permutation key  */
static int permutation_complement_bit = 0;

/* Generate a new sequence of pseudo-random values for the specified seed
   number. If SEED is less than 0, use current time instead.  */

void
srandsec (int seed)
{
  if (seed < 0)
    seed = currentns ();

  permutation_complement_bit = seed % 2;

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
   than a second are permuted.  */
static int
permutens (int nsec, bool random)
{
  int ns = 0;
  int ns_digits[FT_NSEC_DIGITS] = { 0 };
  int i;
  for (i = FT_NSEC_DIGITS - 1; i >= 0; i--)
    {
      ns_digits[i] = nsec % 10;
      nsec /= 10;
    }

  const int permuted_sizes[] = { 720, 120, 24, 6, 2, 1 };
  int permutation_key;
  if (random)
    {
      int r = rand () % (RAND_USE_MAX + 1);
      if (r < 30240)
        permutation_key = r;
      else
        permutation_key = ((r % 5040) << 1) + permutation_complement_bit;
    }
  else
    permutation_key = currentns ();

  /* If PERMUTATION_KEY is greater than the multiple of PERMUTED_SIZES in
     each index, divide the key by the size and move digits forward placed
     in the back position from its index to value.

      KEY  |  SEQUENCE       |  PERMUTATION
     ----- | --------------- | --------------------------------------------
        0  |  1 2 3 4 5 6 7  |  Don't move digits placed in the same index
        1  |  1 2 3 4 5 7 6  |  Move 7 to the index of 5
        2  |  1 2 3 4 6 5 7  |  Move 6 to the index of 4
        3  |  1 2 3 4 6 7 5  |  Move 6 and 7 to the index of 4 and 5
        4  |  1 2 3 4 7 5 6  |  Move 7 to the index of 4
        5  |  1 2 3 4 7 6 5  |  Move 7 and 6 to the index of 4 and 5
        6  |  1 2 3 5 4 6 7  |  Move 5 to the index of 3
        7  |  1 2 3 5 4 7 6  |  Move 5 and 7 to the index of 3 and 5
        8  |  1 2 3 5 6 4 7  |  Move 5 and 6 to the index of 3 and 4      */

  for (i = 0; i < FT_NSEC_DIGITS - 1; i++)
    {
      int moved_forward_index =
            (permutation_key / permuted_sizes[i]) % (FT_NSEC_DIGITS - i) + i;
      if (i ^ moved_forward_index)
        {
          int digit = ns_digits[moved_forward_index];
          int d;
          for (d = moved_forward_index - 1; d >= i; d--)
            ns_digits[d + 1] = ns_digits[d];
          ns_digits[i] = digit;
        }
    }

  for (i = 0; i < FT_NSEC_DIGITS; i++)
     ns = ns * 10 + ns_digits[i];

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
      bool ns_random = IS_NSEC_RANDOM (modflag);

      if (ns && IS_SECONDS_ROUNDING (modflag))
        {
          /* Give priority to round seconds up if both are specified. */
          if (IS_SECONDS_ROUND_UP (modflag)
              && (IMAX_ADD_WRAPV (1, sec, &sec) || timew_overflow (sec)))
            return false;

          ns = 0;
        }

      if (ns_random)
        ns = randns ();

      if (IS_NSEC_PERMUTE (modflag))
        ns = permutens (ns, ! ns_random);

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
  -C        round up to the smallest second that is not less than SECONDS\n\
  -F        round down to the largest second that does not exceed SECONDS\n\
  -P        permute digits in nanoseconds less than a second at random\n\
  -R SEED   set 100 nanoseconds at random by SEED; If 0, use current time",
true, 0);
  exit (status);
}

int
main (int argc, char **argv)
{
  intmax_t seconds;
  int nsec = 0;
  int sec_modflag = 0;
  int repeat_num = 1;
  int seed = 0;
  int c, i;
  int status = EXIT_SUCCESS;
  int set_num;
  char *endptr;
  struct tm_ptrs tm_ptrs = (struct tm_ptrs) { .elapse = &seconds,
                                              .frac_val = &nsec };
  struct tm_fmt tm_fmt = { false };

  while ((c = getopt (argc, argv, ":CFPR:")) != -1)
    {
      switch (c)
        {
        case 'C':
          sec_modflag |= SECONDS_ROUND_UP;
          break;
        case 'F':
          sec_modflag |= SECONDS_ROUND_DOWN;
          break;
        case 'P':
          sec_modflag |= NSEC_PERMUTE;
          break;
        case 'R':
          sec_modflag |= NSEC_RANDOM;
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

  if (IS_SECONDS_ROUND_UP (sec_modflag) && IS_SECONDS_ROUND_DOWN (sec_modflag))
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
  if (IS_NSEC_RANDOMIZING (sec_modflag))
    srandsec (--seed);

  for (i = 0; i < repeat_num; i++)
    {
      intmax_t sec = seconds;
      int ns = nsec;

      tm_ptrs.elapse = &sec;
      tm_ptrs.frac_val = &ns;

      if (! modifysec (&sec, &ns, sec_modflag))
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
