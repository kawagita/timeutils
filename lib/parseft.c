/* Parse a string into parameters of setting file time.

   Copyright (C) 1999-2000, 2002-2025 Free Software Foundation, Inc.
   Copyright (C) 2025 Yoshinori Kawagita.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* Originally written by Steven M. Bellovin <smb@research.att.com> while
   at the University of North Carolina at Chapel Hill.  Later tweaked by
   a couple of people on Usenet.  Completely overhauled by Rich $alz
   <rsalz@bbn.com> and Jim Berets <jberets@bbn.com> in August, 1990.

   Modified by Assaf Gordon <assafgordon@gmail.com> in 2016 to add
   debug output.

   Modified by Paul Eggert <eggert@twinsun.com> in 1999 to do the
   right thing about local DST.  Also modified by Paul Eggert
   <eggert@cs.ucla.edu> in 2004 to support nanosecond-resolution
   timestamps, in 2004 to support TZ strings in dates, and in 2017 and 2020 to
   check for integer overflow and to support longer-than-'long'
   'time_t' and 'tv_nsec'.  */

#include "config.h"

#ifndef USE_TM_GLIBC
# include <windows.h>
#endif
#include <ctype.h>
#include <inttypes.h>
#include <limits.h>
#include <stdbool.h>
#ifdef TEST
# include <stdio.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>

#include "ft.h"
#include "ftsec.h"
#include "idx.h"
#include "imaxoverflow.h"
#include "intoverflow.h"
#include "wintm.h"

#ifndef USE_TM_GLIBC
# include "encword.h"
#endif

/* ISDIGIT differs from isdigit, as follows:
   - Its arg may be any int or unsigned int; it need not be an unsigned char
     or EOF.
   - It's typically faster.
   POSIX says that only '0' through '9' are digits.  Prefer ISDIGIT to
   isdigit unless it's important to use the locale's definition
   of `digit' even when the host does not conform to POSIX.  */
#define ISDIGIT(c) ((unsigned int) (c) - '0' <= 9)

#define HOUR(x) (60 * 60 * (x))

/* Convert a possibly-signed character to an unsigned character.  This is
   a bit safer than casting to unsigned char, since it catches some type
   errors that the cast doesn't.  */
static unsigned char to_uchar (char ch) { return ch; }


/* An integer value, and the number of digits in its textual
   representation.  */
typedef struct
{
  bool negative;
  intmax_t value;
  idx_t digits;
} textint;

/* An entry in the lexical lookup table.  */
typedef struct
{
  char const *name;
  int type;
  int value;
} table;

/* Meridian: am, pm, or 24-hour style.  */
enum { MERam, MERpm, MER24 };

/* Maximum length of a time zone name or abbreviation, plus 1.  */
enum { TIME_ZONE_BUFSIZE = 64 };

/* Relative times.  */
typedef struct
{
  /* Relative year, month, day, hour, minutes, seconds, and nanoseconds.  */
  intmax_t year;
  intmax_t month;
  intmax_t day;
  intmax_t hour;
  intmax_t minutes;
  intmax_t seconds;
  int ns;
} relative_time;

#define RELATIVE_TIME_0 ((relative_time) { 0, 0, 0, 0, 0, 0, 0 })

/* Information passed to and from the parser.  */
typedef struct
{
  /* The input string remaining to be parsed.  */
  const char *input;

  /* N, if this is the Nth Tuesday.  */
  intmax_t day_ordinal;

  /* Day of week; Sunday is 0.  */
  int day_number;

  /* tm_isdst flag for the local zone.  */
  int local_isdst;

  /* Time zone, in seconds east of UT.  */
  int time_zone;

  /* Style used for time.  */
  int meridian;

  /* Gregorian year, month, day, hour, minutes, seconds, and nanoseconds.  */
  textint year;
  intmax_t month;
  intmax_t day;
  intmax_t hour;
  intmax_t minutes;
  intmax_t seconds;
  int nsec;

  /* Relative year, month, day, hour, minutes, seconds, and nanoseconds.  */
  relative_time rel;

  /* Presence or counts of nonterminals of various flavors parsed so far.  */
  bool timespec_seen;
  bool rels_seen;
  idx_t dates_seen;
  idx_t days_seen;
  idx_t J_zones_seen;
  idx_t local_zones_seen;
  idx_t dsts_seen;
  idx_t times_seen;
  idx_t zones_seen;
  bool year_seen;

  /* Table of local time zone abbrevations, terminated by a null entry.  */
  table local_time_zone_table[3];

  /* The abbreviations in LOCAL_TIME_ZONE_TABLE.  */
  char tz_abbr[2][TIME_ZONE_BUFSIZE];

#ifndef USE_TM_GLIBC
  /* ANSI codepage of encoding on Windows.  */
  int ansi_cp;
#endif
} parser_control;

/* Increment PC->rel by FACTOR * REL (FACTOR is 1 or -1).  Return true
   if successful, false if an overflow occurred.  */
static bool
apply_relative_time (parser_control *pc, relative_time rel, int factor)
{
  if (factor < 0
      ? (INT_SUBTRACT_WRAPV (pc->rel.ns, rel.ns, &pc->rel.ns)
         | IMAX_SUBTRACT_WRAPV (pc->rel.seconds, rel.seconds, &pc->rel.seconds)
         | IMAX_SUBTRACT_WRAPV (pc->rel.minutes, rel.minutes, &pc->rel.minutes)
         | IMAX_SUBTRACT_WRAPV (pc->rel.hour, rel.hour, &pc->rel.hour)
         | IMAX_SUBTRACT_WRAPV (pc->rel.day, rel.day, &pc->rel.day)
         | IMAX_SUBTRACT_WRAPV (pc->rel.month, rel.month, &pc->rel.month)
         | IMAX_SUBTRACT_WRAPV (pc->rel.year, rel.year, &pc->rel.year))
      : (INT_ADD_WRAPV (pc->rel.ns, rel.ns, &pc->rel.ns)
         | IMAX_ADD_WRAPV (pc->rel.seconds, rel.seconds, &pc->rel.seconds)
         | IMAX_ADD_WRAPV (pc->rel.minutes, rel.minutes, &pc->rel.minutes)
         | IMAX_ADD_WRAPV (pc->rel.hour, rel.hour, &pc->rel.hour)
         | IMAX_ADD_WRAPV (pc->rel.day, rel.day, &pc->rel.day)
         | IMAX_ADD_WRAPV (pc->rel.month, rel.month, &pc->rel.month)
         | IMAX_ADD_WRAPV (pc->rel.year, rel.year, &pc->rel.year)))
    return false;
  pc->rels_seen = true;
  return true;
}

/* Set PC-> hour, minutes, seconds and nanoseconds members from arguments.  */
static void
set_hhmmss (parser_control *pc, intmax_t hour, intmax_t minutes,
            intmax_t sec, int nsec)
{
  pc->hour = hour;
  pc->minutes = minutes;
  pc->seconds = sec;
  pc->nsec = nsec;
}


/* Token kinds.  */
enum
{
  tAGO = 258,                    /* tAGO  */
  tDST = 259,                    /* tDST  */
  tYEAR_UNIT = 260,              /* tYEAR_UNIT  */
  tMONTH_UNIT = 261,             /* tMONTH_UNIT  */
  tHOUR_UNIT = 262,              /* tHOUR_UNIT  */
  tMINUTE_UNIT = 263,            /* tMINUTE_UNIT  */
  tSEC_UNIT = 264,               /* tSEC_UNIT  */
  tDAY_UNIT = 265,               /* tDAY_UNIT  */
  tDAY_SHIFT = 266,              /* tDAY_SHIFT  */
  tDAY = 267,                    /* tDAY  */
  tDAYZONE = 268,                /* tDAYZONE  */
  tLOCAL_ZONE = 269,             /* tLOCAL_ZONE  */
  tMERIDIAN = 270,               /* tMERIDIAN  */
  tMONTH = 271,                  /* tMONTH  */
  tORDINAL = 272,                /* tORDINAL  */
  tZONE = 273,                   /* tZONE  */
  tSNUMBER = 274,                /* tSNUMBER  */
  tUNUMBER = 275,                /* tUNUMBER  */
  tSDECIMAL_NUMBER = 276,        /* tSDECIMAL_NUMBER  */
  tUDECIMAL_NUMBER = 277         /* tUDECIMAL_NUMBER  */
};

/* Value type.  */
union YYSTYPE
{
  intmax_t intval;
  textint textintval;
  struct {
    intmax_t seconds;
    int nsec;
  } timespec;
  relative_time rel;
};
typedef union YYSTYPE YYSTYPE;

static table const meridian_table[] =
{
  { "AM",   tMERIDIAN, MERam },
  { "A.M.", tMERIDIAN, MERam },
  { "PM",   tMERIDIAN, MERpm },
  { "P.M.", tMERIDIAN, MERpm },
  { NULL, 0, 0 }
};

static table const dst_table[] =
{
  { "DST", tDST, 0 }
};

static table const month_and_day_table[] =
{
  { "JANUARY",  tMONTH,  1 },
  { "FEBRUARY", tMONTH,  2 },
  { "MARCH",    tMONTH,  3 },
  { "APRIL",    tMONTH,  4 },
  { "MAY",      tMONTH,  5 },
  { "JUNE",     tMONTH,  6 },
  { "JULY",     tMONTH,  7 },
  { "AUGUST",   tMONTH,  8 },
  { "SEPTEMBER",tMONTH,  9 },
  { "SEPT",     tMONTH,  9 },
  { "OCTOBER",  tMONTH, 10 },
  { "NOVEMBER", tMONTH, 11 },
  { "DECEMBER", tMONTH, 12 },
  { "SUNDAY",   tDAY,    0 },
  { "MONDAY",   tDAY,    1 },
  { "TUESDAY",  tDAY,    2 },
  { "TUES",     tDAY,    2 },
  { "WEDNESDAY",tDAY,    3 },
  { "WEDNES",   tDAY,    3 },
  { "THURSDAY", tDAY,    4 },
  { "THUR",     tDAY,    4 },
  { "THURS",    tDAY,    4 },
  { "FRIDAY",   tDAY,    5 },
  { "SATURDAY", tDAY,    6 },
  { NULL, 0, 0 }
};

static table const time_units_table[] =
{
  { "YEAR",     tYEAR_UNIT,      1 },
  { "MONTH",    tMONTH_UNIT,     1 },
  { "FORTNIGHT",tDAY_UNIT,      14 },
  { "WEEK",     tDAY_UNIT,       7 },
  { "DAY",      tDAY_UNIT,       1 },
  { "HOUR",     tHOUR_UNIT,      1 },
  { "MINUTE",   tMINUTE_UNIT,    1 },
  { "MIN",      tMINUTE_UNIT,    1 },
  { "SECOND",   tSEC_UNIT,       1 },
  { "SEC",      tSEC_UNIT,       1 },
  { NULL, 0, 0 }
};

/* Assorted relative-time words.  */
static table const relative_time_table[] =
{
  { "TOMORROW", tDAY_SHIFT,      1 },
  { "YESTERDAY",tDAY_SHIFT,     -1 },
  { "TODAY",    tDAY_SHIFT,      0 },
  { "NOW",      tDAY_SHIFT,      0 },
  { "LAST",     tORDINAL,       -1 },
  { "THIS",     tORDINAL,        0 },
  { "NEXT",     tORDINAL,        1 },
  { "FIRST",    tORDINAL,        1 },
/*{ "SECOND",   tORDINAL,        2 }, */
  { "THIRD",    tORDINAL,        3 },
  { "FOURTH",   tORDINAL,        4 },
  { "FIFTH",    tORDINAL,        5 },
  { "SIXTH",    tORDINAL,        6 },
  { "SEVENTH",  tORDINAL,        7 },
  { "EIGHTH",   tORDINAL,        8 },
  { "NINTH",    tORDINAL,        9 },
  { "TENTH",    tORDINAL,       10 },
  { "ELEVENTH", tORDINAL,       11 },
  { "TWELFTH",  tORDINAL,       12 },
  { "AGO",      tAGO,           -1 },
  { "HENCE",    tAGO,            1 },
  { NULL, 0, 0 }
};

/* The universal time zone table.  These labels can be used even for
   timestamps that would not otherwise be valid, e.g., GMT timestamps
   oin London during summer.  */
static table const universal_time_zone_table[] =
{
  { "GMT",      tZONE,     HOUR ( 0) }, /* Greenwich Mean */
  { "UT",       tZONE,     HOUR ( 0) }, /* Universal (Coordinated) */
  { "UTC",      tZONE,     HOUR ( 0) },
  { NULL, 0, 0 }
};

/* The time zone table.  This table is necessarily incomplete, as time
   zone abbreviations are ambiguous; e.g., Australians interpret "EST"
   as Eastern time in Australia, not as US Eastern Standard Time.
   You cannot rely on parse_datetime to handle arbitrary time zone
   abbreviations; use numeric abbreviations like "-0500" instead.  */
static table const time_zone_table[] =
{
  { "WET",      tZONE,     HOUR ( 0) }, /* Western European */
  { "WEST",     tDAYZONE,  HOUR ( 0) }, /* Western European Summer */
  { "BST",      tDAYZONE,  HOUR ( 0) }, /* British Summer */
  { "ART",      tZONE,    -HOUR ( 3) }, /* Argentina */
  { "BRT",      tZONE,    -HOUR ( 3) }, /* Brazil */
  { "BRST",     tDAYZONE, -HOUR ( 3) }, /* Brazil Summer */
  { "NST",      tZONE,   -(HOUR ( 3) + 30 * 60) }, /* Newfoundland Standard */
  { "NDT",      tDAYZONE,-(HOUR ( 3) + 30 * 60) }, /* Newfoundland Daylight */
  { "AST",      tZONE,    -HOUR ( 4) }, /* Atlantic Standard */
  { "ADT",      tDAYZONE, -HOUR ( 4) }, /* Atlantic Daylight */
  { "CLT",      tZONE,    -HOUR ( 4) }, /* Chile */
  { "CLST",     tDAYZONE, -HOUR ( 4) }, /* Chile Summer */
  { "EST",      tZONE,    -HOUR ( 5) }, /* Eastern Standard */
  { "EDT",      tDAYZONE, -HOUR ( 5) }, /* Eastern Daylight */
  { "CST",      tZONE,    -HOUR ( 6) }, /* Central Standard */
  { "CDT",      tDAYZONE, -HOUR ( 6) }, /* Central Daylight */
  { "MST",      tZONE,    -HOUR ( 7) }, /* Mountain Standard */
  { "MDT",      tDAYZONE, -HOUR ( 7) }, /* Mountain Daylight */
  { "PST",      tZONE,    -HOUR ( 8) }, /* Pacific Standard */
  { "PDT",      tDAYZONE, -HOUR ( 8) }, /* Pacific Daylight */
  { "AKST",     tZONE,    -HOUR ( 9) }, /* Alaska Standard */
  { "AKDT",     tDAYZONE, -HOUR ( 9) }, /* Alaska Daylight */
  { "HST",      tZONE,    -HOUR (10) }, /* Hawaii Standard */
  { "HAST",     tZONE,    -HOUR (10) }, /* Hawaii-Aleutian Standard */
  { "HADT",     tDAYZONE, -HOUR (10) }, /* Hawaii-Aleutian Daylight */
  { "SST",      tZONE,    -HOUR (12) }, /* Samoa Standard */
  { "WAT",      tZONE,     HOUR ( 1) }, /* West Africa */
  { "CET",      tZONE,     HOUR ( 1) }, /* Central European */
  { "CEST",     tDAYZONE,  HOUR ( 1) }, /* Central European Summer */
  { "MET",      tZONE,     HOUR ( 1) }, /* Middle European */
  { "MEZ",      tZONE,     HOUR ( 1) }, /* Middle European */
  { "MEST",     tDAYZONE,  HOUR ( 1) }, /* Middle European Summer */
  { "MESZ",     tDAYZONE,  HOUR ( 1) }, /* Middle European Summer */
  { "EET",      tZONE,     HOUR ( 2) }, /* Eastern European */
  { "EEST",     tDAYZONE,  HOUR ( 2) }, /* Eastern European Summer */
  { "CAT",      tZONE,     HOUR ( 2) }, /* Central Africa */
  { "SAST",     tZONE,     HOUR ( 2) }, /* South Africa Standard */
  { "EAT",      tZONE,     HOUR ( 3) }, /* East Africa */
  { "MSK",      tZONE,     HOUR ( 3) }, /* Moscow */
  { "MSD",      tDAYZONE,  HOUR ( 3) }, /* Moscow Daylight */
  { "IST",      tZONE,    (HOUR ( 5) + 30 * 60) }, /* India Standard */
  { "SGT",      tZONE,     HOUR ( 8) }, /* Singapore */
  { "KST",      tZONE,     HOUR ( 9) }, /* Korea Standard */
  { "JST",      tZONE,     HOUR ( 9) }, /* Japan Standard */
  { "GST",      tZONE,     HOUR (10) }, /* Guam Standard */
  { "NZST",     tZONE,     HOUR (12) }, /* New Zealand Standard */
  { "NZDT",     tDAYZONE,  HOUR (12) }, /* New Zealand Daylight */
  { NULL, 0, 0 }
};

/* Military time zone table.

   RFC 822 got these backwards, but RFC 5322 makes the incorrect
   treatment optional, so do them the right way here.

   'J' is special, as it is local time.
   'T' is also special, as it is the separator in ISO
   8601 date and time of day representation.  */
static table const military_table[] =
{
  { "A", tZONE,  HOUR ( 1) },
  { "B", tZONE,  HOUR ( 2) },
  { "C", tZONE,  HOUR ( 3) },
  { "D", tZONE,  HOUR ( 4) },
  { "E", tZONE,  HOUR ( 5) },
  { "F", tZONE,  HOUR ( 6) },
  { "G", tZONE,  HOUR ( 7) },
  { "H", tZONE,  HOUR ( 8) },
  { "I", tZONE,  HOUR ( 9) },
  { "J", 'J',    0 },
  { "K", tZONE,  HOUR (10) },
  { "L", tZONE,  HOUR (11) },
  { "M", tZONE,  HOUR (12) },
  { "N", tZONE, -HOUR ( 1) },
  { "O", tZONE, -HOUR ( 2) },
  { "P", tZONE, -HOUR ( 3) },
  { "Q", tZONE, -HOUR ( 4) },
  { "R", tZONE, -HOUR ( 5) },
  { "S", tZONE, -HOUR ( 6) },
  { "T", 'T',    0 },
  { "U", tZONE, -HOUR ( 8) },
  { "V", tZONE, -HOUR ( 9) },
  { "W", tZONE, -HOUR (10) },
  { "X", tZONE, -HOUR (11) },
  { "Y", tZONE, -HOUR (12) },
  { "Z", tZONE,  HOUR ( 0) },
  { NULL, 0, 0 }
};



/* Convert a time zone expressed as HH:MM into an integer count of
   seconds.  If MM is negative, then S is of the form HHMM and needs
   to be picked apart; otherwise, S is of the form HH.  As specified in
   https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/V1_chap08.html#tag_08_03, allow
   only valid TZ range, and consider first two digits as hours, if no
   minutes specified.  Return true if successful.  */

static bool
time_zone_hhmm (parser_control *pc, textint s, intmax_t mm)
{
  intmax_t n_minutes;
  bool overflow = false;

  /* If the length of S is 1 or 2 and no minutes are specified,
     interpret it as a number of hours.  */
  if (s.digits <= 2 && mm < 0)
    s.value *= 100;

  if (mm < 0)
    n_minutes = (s.value / 100) * 60 + s.value % 100;
  else
    {
      overflow |= IMAX_MULTIPLY_WRAPV (s.value, 60, &n_minutes);
      overflow |= (s.negative
                   ? IMAX_SUBTRACT_WRAPV (n_minutes, mm, &n_minutes)
                   : IMAX_ADD_WRAPV (n_minutes, mm, &n_minutes));
    }

  if (overflow || ! (-24 * 60 <= n_minutes && n_minutes <= 24 * 60))
    return false;
  pc->time_zone = n_minutes * 60;
  return true;
}

static int
to_hour (intmax_t hours, int meridian)
{
  switch (meridian)
    {
    default: /* Pacify GCC.  */
    case MER24:
      return 0 <= hours && hours < 24 ? hours : -1;
    case MERam:
      return 0 < hours && hours < 12 ? hours : hours == 12 ? 0 : -1;
    case MERpm:
      return 0 < hours && hours < 12 ? hours + 12 : hours == 12 ? 12 : -1;
    }
}

/* Convert a text year number to a year minus 1900, working correctly
   even if the input is in the range INT_MAX .. INT_MAX + 1900 - 1.  */

static bool
to_tm_year (textint textyear, int *tm_year)
{
  intmax_t year = textyear.value;

  /* XPG4 suggests that years 00-68 map to 2000-2068, and
     years 69-99 map to 1969-1999.  */
  if (0 <= year && textyear.digits == 2)
    year += year < 69 ? 2000 : 1900;

  if (year <= INT_MIN || year > INT_MAX
      || year < 0
         ? INT_SUBTRACT_WRAPV (-TM_YEAR_BASE, year, tm_year)
         : INT_SUBTRACT_WRAPV (year, TM_YEAR_BASE, tm_year))
    return false;

  return true;
}

static table const *
lookup_zone (parser_control const *pc, char const *name)
{
  table const *tp;

  for (tp = universal_time_zone_table; tp->name; tp++)
    if (strcmp (name, tp->name) == 0)
      return tp;

  /* Try local zone abbreviations before those in time_zone_table, as
     the local ones are more likely to be right.  */
  for (tp = pc->local_time_zone_table; tp->name; tp++)
    if (strcmp (name, tp->name) == 0)
      return tp;

  for (tp = time_zone_table; tp->name; tp++)
    if (strcmp (name, tp->name) == 0)
      return tp;

  return NULL;
}

static table const *
lookup_word (parser_control const *pc, char *word)
{
  char *p;
  char *q;
  idx_t wordlen;
  table const *tp;
  bool period_found;
  bool abbrev;

  /* Make it uppercase if alphabet characters.  */
  if (isalpha (*word))
    for (p = word; *p; p++)
      *p = toupper (to_uchar (*p));

  for (tp = meridian_table; tp->name; tp++)
    if (strcmp (word, tp->name) == 0)
      return tp;

  /* See if we have an abbreviation for a month.  */
  wordlen = strlen (word);
  abbrev = wordlen == 3 || (wordlen == 4 && word[3] == '.');

  for (tp = month_and_day_table; tp->name; tp++)
    if ((abbrev ? strncmp (word, tp->name, 3) : strcmp (word, tp->name)) == 0)
      return tp;

  if ((tp = lookup_zone (pc, word)))
    return tp;

  if (strcmp (word, dst_table[0].name) == 0)
    return dst_table;

  for (tp = time_units_table; tp->name; tp++)
    if (strcmp (word, tp->name) == 0)
      return tp;

  /* Strip off any plural and try the units table again.  */
  if (word[wordlen - 1] == 'S')
    {
      word[wordlen - 1] = '\0';
      for (tp = time_units_table; tp->name; tp++)
        if (strcmp (word, tp->name) == 0)
          return tp;
      word[wordlen - 1] = 'S';  /* For "this" in relative_time_table.  */
    }

  for (tp = relative_time_table; tp->name; tp++)
    if (strcmp (word, tp->name) == 0)
      return tp;

  /* Military time zones.  */
  if (wordlen == 1)
    for (tp = military_table; tp->name; tp++)
      if (word[0] == tp->name[0])
        return tp;

  /* Drop out any periods and try the time zone table again.  */
  for (period_found = false, p = q = word; (*p = *q); q++)
    if (*q == '.')
      period_found = true;
    else
      p++;
  if (period_found && (tp = lookup_zone (pc, word)))
    return tp;

  return NULL;
}

static int
yylex (YYSTYPE *lvalp, parser_control *pc)
{
  unsigned char c;

  for (;;)
    {
      while (c = *pc->input, isspace (c))
        pc->input++;

      if (ISDIGIT (c) || c == '-' || c == '+')
        {
          char const *p = pc->input;
          int sign;
          if (c == '-' || c == '+')
            {
              sign = c == '-' ? -1 : 1;
              while (c = *(pc->input = ++p), isspace (c))
                continue;
              if (! ISDIGIT (c))
                /* skip the '-' sign */
                continue;
            }
          else
            sign = 0;

          intmax_t value = 0;
          do
            {
              if (IMAX_MULTIPLY_WRAPV (value, 10, &value))
                return '?';
              if (IMAX_ADD_WRAPV (value, sign < 0 ? '0' - c : c - '0', &value))
                return '?';
              c = *++p;
            }
          while (ISDIGIT (c));

          if ((c == '.' || c == ',') && ISDIGIT (p[1]))
            {
              int digits;

              /* Accumulate fraction, to ns precision.  */
              p++;
              int ns = *p++ - '0';
              for (digits = 2; digits <= FT_NSEC_DIGITS; digits++)
                {
                  ns *= 10;
                  if (ISDIGIT (*p))
                    ns += *p++ - '0';
                }

              /* Skip excess digits, truncating toward -Infinity.  */
              if (sign < 0)
                for (; ISDIGIT (*p); p++)
                  if (*p != '0')
                    {
                      ns++;
                      break;
                    }
              while (ISDIGIT (*p))
                p++;

              /* Adjust to the timespec convention, which is that
                 tv_nsec is always a positive offset even if tv_sec is
                 negative.  */
              if (sign < 0 && ns)
                {
                  if (IMAX_SUBTRACT_WRAPV (value, 1, &value))
                    return '?';
                  ns = FT_NSEC_PRECISION - ns;
                }

              lvalp->timespec.seconds = value;
              lvalp->timespec.nsec = ns;
              pc->input = p;
              return sign ? tSDECIMAL_NUMBER : tUDECIMAL_NUMBER;
            }
          else
            {
              lvalp->textintval.negative = sign < 0;
              lvalp->textintval.value = value;
              lvalp->textintval.digits = p - pc->input;
              pc->input = p;
              return sign ? tSNUMBER : tUNUMBER;
            }
        }

#ifndef USE_TM_GLIBC
      if (ispunct (c) || c == '\0')
#else
      if (! isalpha (c))
#endif
        {
          if (c != '(')
            return to_uchar (*pc->input++);
        }
      else
        {
#ifndef USE_TM_GLIBC
          char buff[TIME_ZONE_BUFSIZE];
          char *p = buff;
          int maxsize = TIME_ZONE_BUFSIZE;
          bool alpha_input = false;

          if (isalpha (c))
            {
              do
                {
                  *p++ = c;
                  c = *++pc->input;
                  maxsize--;
                }
              while (isalpha (c) || c == '.');

              if (isspace (c) || ispunct(c) || ISDIGIT (c))
                {
                  *p = '\0';
                  alpha_input = true;
                }
            }

          if (! alpha_input)
            pc->input += encword (p, maxsize, pc->input, pc->ansi_cp);
#else
          char buff[20];
          char *p = buff;

          do
            {
              if (p < buff + sizeof buff - 1)
                *p++ = c;
              c = *++pc->input;
            }
          while (isalpha (c) || c == '.');

          *p = '\0';
#endif

          table const *tp = lookup_word (pc, buff);
          if (! tp)
            return '?';
          lvalp->intval = tp->value;
          return tp->type;
        }

      idx_t count = 0;
      do
        {
          c = *pc->input++;
          if (c == '\0')
            return c;
          if (c == '(')
            count++;
          else if (c == ')')
            count--;
        }
      while (count != 0);
    }
  return 1;
}

/* States of parsing an expression.  */
enum
{
  STATE_START = 0,
  STATE_ACCEPT = 1,
  STATE_ABORT = 2,
  STATE_STOP = 3
};

#ifdef TEST
/* Set true if the state of parsing an expression is output.  */
static bool parsing_output = false;

static char const *parsing_states[] = { "Start", "Accept", "Abort", "Stop" };

/* Output the specified state of parsing an expression.  */
static inline int
print_parsing_state (int state, int nest, char const *symbol, char const *expr)
{
  printf (" %-6s | %s", parsing_states[state], symbol);
  if (nest > 0)
    printf ("[%d]", nest);
  switch (state)
    {
    case STATE_START:
      printf (" ? \"%s\"", expr);
      break;
    case STATE_ACCEPT:
    case STATE_ABORT:
      printf (" := %s", expr);
      break;
    case STATE_STOP:
      break;
    }
  fputs ("\n", stdout);
  return 1;
}

/* Token names of relunit.  */
static char const *relunit_names[] =
{
  "tYEAR_UNIT",
  "tMONTH_UNIT",
  "tHOUR_UNIT",
  "tMINUTE_UNIT",
  "tSEC_UNIT",
  "tDAY_UNIT"
};

/* Token names of ordinal or number.  */
static char const *number_names[] =
{
  "tORDINAL",
  "",  /* tZONE */
  "tSNUMBER",
  "tUNUMBER",
  "tSDECIMAL_NUMBER",
  "tUDECIMAL_NUMBER"
};

/* Output the specified state of parsing a relunit.  */
static const int
print_parsing_relunit_state (int state, char const *symbol,
                             int num_token, int relunit_token)
{
  printf (" %-6s | %s :=", parsing_states[state], symbol);
  if (num_token >= tORDINAL && num_token <= tUDECIMAL_NUMBER)
    printf (" %s", number_names[num_token - tORDINAL]);
  if (relunit_token >= tYEAR_UNIT && relunit_token <= tDAY_UNIT)
    printf (" %s", relunit_names[relunit_token - tYEAR_UNIT]);
  fputs ("\n", stdout);
  return 1;
}

/* Output the state of parsing an expression.  */
# define PARSING_EXPR(state,symbol,expr) \
           (parsing_output ? print_parsing_state (state, 0, symbol, expr) : 0)
# define PARSING_RELUNIT(state,symbol,num_token,relunit_token) \
           (parsing_output \
            ? print_parsing_relunit_state ( \
                state, symbol, num_token, relunit_token) : 0)
# define PARSING_START(symbol,expr)  PARSING_EXPR (STATE_START, symbol, expr)
# define PARSING_ACCEPT(symbol,expr) PARSING_EXPR (STATE_ACCEPT, symbol, expr)
# define PARSING_ACCEPT_RELUNIT(symbol,num_token,relunit_token) \
           PARSING_RELUNIT (STATE_ACCEPT, symbol, num_token, relunit_token)
# define PARSING_ACCEPT_NUMBER(symbol,num_token) \
           PARSING_RELUNIT (STATE_ACCEPT, symbol, num_token, -1)
# define PARSING_ABORT(symbol,expr)  PARSING_EXPR (STATE_ABORT, symbol, expr)
# define PARSING_ABORT_RELUNIT(symbol,num_token,relunit_token) \
           PARSING_RELUNIT (STATE_ABORT, symbol, num_token, relunit_token)
# define PARSING_ABORT_NUMBER(symbol,num_token) \
           PARSING_RELUNIT (STATE_ABORT, symbol, num_token, -1)
# define PARSING_STOP(symbol)        PARSING_EXPR (STATE_STOP, symbol, NULL)
#else
# define PARSING_START(symbol,expr)
# define PARSING_ACCEPT(symbol,expr)
# define PARSING_ACCEPT_RELUNIT(symbol,num_token,relunit_token)
# define PARSING_ACCEPT_NUMBER(symbol,num_token)
# define PARSING_ABORT(symbol,expr)
# define PARSING_ABORT_RELUNIT(symbol,num_token,relunit_token)
# define PARSING_ABORT_NUMBER(symbol,num_token)
# define PARSING_STOP(symbol)
#endif

/* Return true if parsing is stopped for STATE and the next is started, and
   increment *COUNTER if not NULL and the specified expression is accepted.  */
static inline bool
next_parsing (int state, char const *symbol, char const *expr, idx_t *counter)
{
  switch (state)
    {
    case STATE_ACCEPT:
      if (counter)
        (*counter)++;
      PARSING_ACCEPT (symbol, expr);
    case STATE_ABORT:
      return false;
    }
  return true;
}

/* Parse the leading string as seconds.

   seconds := signed_seconds | unsigned_seconds;

   signed_seconds   := tSDECIMAL_NUMBER | tSNUMBER
   unsigned_seconds := tUDECIMAL_NUMBER | tUNUMBER

   If accepted as seconds, return STATE_ACCEPT, or if its value is incorrect,
   return STATE_ABORT, otherwise, return STATE_STOP.  */

static int
parse_seconds (YYSTYPE *lvalp, parser_control *pc, bool sign_accepted)
{
  char const *p0 = pc->input;
  YYSTYPE val0 = *lvalp;

  PARSING_START ("seconds", p0);

  int seconds;
  int token = yylex (lvalp, pc);
  switch (token)
    {
    case tSDECIMAL_NUMBER:
      if (! sign_accepted)
        break;

    case tUDECIMAL_NUMBER:
      /* Set seconds and nanoseconds into lvalp->timespec by lex function. */
      PARSING_ACCEPT_NUMBER ("seconds", token);
      return STATE_ACCEPT;

    case tSNUMBER:
      if (! sign_accepted)
        break;

    case tUNUMBER:
      seconds = lvalp->textintval.value;
      if (! secoverflow (seconds, 0))
        {
          lvalp->timespec.seconds = seconds;
          lvalp->timespec.nsec = 0;

          PARSING_ACCEPT_NUMBER ("seconds", token);
          return STATE_ACCEPT;
        }

      PARSING_ABORT_NUMBER ("seconds", token);
      return STATE_ABORT;
    }

  pc->input = p0;
  *lvalp = val0;

  PARSING_STOP ("seconds");
  return STATE_STOP;
}

/* Parse the leading string as timespec.

   timespec := '@' seconds

   If accepted as timespec, return STATE_ACCEPT, or if its value is incorrect,
   return STATE_ABORT, otherwise, return STATE_STOP.  */

static int
parse_timespec (YYSTYPE *lvalp, parser_control *pc)
{
  char const *p0 = pc->input;
  YYSTYPE val0 = *lvalp;

  PARSING_START ("timespec", p0);

  int token = yylex (lvalp, pc);
  if (token == '@')
    {
      switch (parse_seconds (lvalp, pc, true))
        {
        case STATE_ACCEPT:
          pc->seconds = lvalp->timespec.seconds;
          pc->nsec = lvalp->timespec.nsec;
          pc->timespec_seen = true;

          PARSING_ACCEPT ("timespec", "'@' seconds");
          return STATE_ACCEPT;
        case STATE_ABORT:
          return STATE_ABORT;
        }
    }

  pc->input = p0;
  *lvalp = val0;

  PARSING_STOP ("timespec");
  return STATE_STOP;
}

/* Parse the leading string as o_colon_minutes.

   o_colon_minutes := empty | ':' tUNUMBER

   If accepted as o_colon_minutes, return STATE_ACCEPT, or if its value is
   incorrect, return STATE_ABORT, otherwise, return STATE_STOP.  */

static int
parse_o_colon_minutes (YYSTYPE *lvalp, parser_control *pc)
{
  char const *p0 = pc->input;

  PARSING_START ("o_colon_minutes", p0);

  int token = yylex (lvalp, pc);
  if (token == ':')
    {
      token = yylex (lvalp, pc);
      if (token == tUNUMBER)  /* ':' tUNUMBER */
        {
          lvalp->intval = lvalp->textintval.value;

          PARSING_ACCEPT ("o_colon_minutes:", "':' tUNUMBER");
          return STATE_ACCEPT;
        }
    }

  /* empty */
  lvalp->intval = -1;

  pc->input = p0;

  PARSING_ACCEPT ("o_colon_minutes", "empty");
  return STATE_ACCEPT;
}

static int parse_iso_8601_date (YYSTYPE *lvalp, parser_control *pc);
static int parse_iso_8601_time (YYSTYPE *lvalp, parser_control *pc);

/* Parse the leading string as iso_8601_datetime.

   iso_8601_datetime := iso_8601_date 'T' iso_8601_time

   If accepted as iso_8601_datetime, return STATE_ACCEPT, or if its value is
   incorrect, return STATE_ABORT, otherwise, return STATE_STOP.  */

static int
parse_iso_8601_datetime (YYSTYPE *lvalp, parser_control *pc)
{
  char const *p0 = pc->input;
  YYSTYPE val0 = *lvalp;
  int token;

  PARSING_START ("iso_8601_datetime", p0);

  switch (parse_iso_8601_date (lvalp, pc))
    {
    case STATE_ACCEPT:
      token = yylex (lvalp, pc);
      if (token == 'T')
        {
          /* iso_8601_date 'T' iso_8601_time */
          int state = parse_iso_8601_time (lvalp, pc);
          if (! next_parsing (state, "iso_8601_datetime",
                              "iso_8601_date 'T' iso_8601_time", NULL))
            return state;
        }
      break;
    case STATE_ABORT:
      return STATE_ABORT;
    }

  pc->input = p0;
  *lvalp = val0;

  PARSING_STOP ("iso_8601_datetime");
  return STATE_STOP;
}

/* Parse the leading string as datetime.

   datetime := iso_8601_datetime

   If accepted as datetime, return STATE_ACCEPT, or if its value is incorrect,
   return STATE_ABORT, otherwise, return STATE_STOP.  */

static int
parse_datetime (YYSTYPE *lvalp, parser_control *pc)
{
  PARSING_START ("datetime", pc->input);

  /* iso_8601_datetime */
  int state = parse_iso_8601_datetime (lvalp, pc);
  if (! next_parsing (state, "datetime", "iso_8601_datetime", NULL))
    return state;

  PARSING_STOP ("datetime");
  return STATE_STOP;
}

/* Parse the leading string as zone_offset.

   zone_offset := tSNUMBER o_colon_minutes

   If accepted as zone_offset, return STATE_ACCEPT, or if its value is
   incorrect, return STATE_ABORT, otherwise, return STATE_STOP.  */

static int
parse_zone_offset (YYSTYPE *lvalp, parser_control *pc)
{
  char const *p0 = pc->input;
  YYSTYPE val0 = *lvalp;

  PARSING_START ("zone_offset", p0);

  int token = yylex (lvalp, pc);
  if (token == tSNUMBER)
    {
      YYSTYPE val1 = *lvalp;
      switch (parse_o_colon_minutes (lvalp, pc))
        {
        case STATE_ACCEPT:
          pc->zones_seen++;
          if (time_zone_hhmm (pc, val1.textintval, lvalp->intval))
            /* tSNUMBER o_colon_minutes */
            {
              PARSING_ACCEPT ("zone_offset", "tSNUMBER o_colon_minutes");
              return STATE_ACCEPT;
            }

          PARSING_ABORT ("zone_offset", "tSNUMBER o_colon_minutes");
        case STATE_ABORT:
          return STATE_ABORT;
        }
    }

  pc->input = p0;
  *lvalp = val0;

  PARSING_STOP ("zone_offset");
  return STATE_STOP;
}

/* Parse the leading string as o_zone_offset.

   o_zone_offset := empty | zone_offset

   If accepted as o_zone_offset, return STATE_ACCEPT, or if its value is
   incorrect, return STATE_ABORT, otherwise, return STATE_STOP.  */

static int
parse_o_zone_offset (YYSTYPE *lvalp, parser_control *pc)
{
  PARSING_START ("o_zone_offset", pc->input);

  /* zone_offset */
  int state = parse_zone_offset (lvalp, pc);
  if (! next_parsing (state, "o_zone_offset", "zone_offset", NULL))
    return state;

  /* empty */

  PARSING_ACCEPT ("o_zone_offset", "empty");
  return STATE_ACCEPT;
}

/* Parse the leading string as iso_8601_time.

   iso_8601_time := tUNUMBER ':' tUNUMBER ':' unsigned_seconds o_zone_offset
                  | tUNUMBER ':' tUNUMBER o_zone_offset
                  | tUNUMBER zone_offset

   If accepted as iso_8601_time, return STATE_ACCEPT, or if its value is
   incorrect, return STATE_ABORT, otherwise, return STATE_STOP.  */

static int
parse_iso_8601_time (YYSTYPE *lvalp, parser_control *pc)
{
  char const *p0 = pc->input;
  YYSTYPE val0 = *lvalp;

  PARSING_START ("iso_8601_time", p0);

  int token = yylex (lvalp, pc);
  if (token != tUNUMBER)
    {
      pc->input = p0;
      *lvalp = val0;

      PARSING_STOP ("iso_8601_time");
      return STATE_STOP;
    }

  char const *p1 = pc->input;
  YYSTYPE val1 = *lvalp;
  token = yylex (lvalp, pc);
  if (token == ':')
    {
      token = yylex (lvalp, pc);
      if (token == tUNUMBER)
        {
          char const *p3 = pc->input;
          YYSTYPE val3 = *lvalp;
          token = yylex (lvalp, pc);
          if (token == ':')
            {
              YYSTYPE val5;
              int state = parse_seconds (lvalp, pc, false);
              if (state == STATE_ACCEPT)
                {
                  val5 = *lvalp;
                  switch (parse_o_zone_offset (lvalp, pc))
                    {
                    case STATE_ACCEPT:  /* tUNUMBER ':' tUNUMBER ':'
                                           unsigned_seconds o_zone_offset */
                      set_hhmmss (
                        pc, val1.textintval.value, val3.textintval.value,
                        val5.timespec.seconds, val5.timespec.nsec);
                      pc->meridian = MER24;

                      PARSING_ACCEPT ("iso_8601_time",
                                      "tUNUMBER ':' tUNUMBER ':' " \
                                      "unsigned_seconds o_zone_offset");
                      return STATE_ACCEPT;
                    case STATE_ABORT:
                      return STATE_ABORT;
                    }
                }
              else if (state == STATE_ABORT)
                return STATE_ABORT;
            }
          else
            {
              pc->input = p3;
              *lvalp = val3;
              switch (parse_o_zone_offset (lvalp, pc))
                {
                case STATE_ACCEPT:  /* tUNUMBER ':' tUNUMBER o_zone_offset */
                  set_hhmmss (
                    pc, val1.textintval.value, val3.textintval.value, 0, 0);
                  pc->meridian = MER24;

                  PARSING_ACCEPT ("iso_8601_time",
                                  "tUNUMBER ':' tUNUMBER o_zone_offset");
                  return STATE_ACCEPT;
                case STATE_ABORT:
                  return STATE_ABORT;
                }
            }
        }
    }
  else
    {
      pc->input = p1;
      *lvalp = val1;
      switch (parse_zone_offset (lvalp, pc))
        {
        case STATE_ACCEPT:  /* tUNUMBER zone_offset */
          set_hhmmss (pc, val1.textintval.value, 0, 0, 0);
          pc->meridian = MER24;

          PARSING_ACCEPT ("iso_8601_time", "tUNUMBER zone_offset");
          return STATE_ACCEPT;
        case STATE_ABORT:
          return STATE_ABORT;
        }
    }

  pc->input = p0;
  *lvalp = val0;

  PARSING_STOP ("iso_8601_time");
  return STATE_STOP;
}

/* Parse the leading string as time.

   time := tUNUMBER tMERIDIAN
         | tUNUMBER ':' tUNUMBER tMERIDIAN
         | tUNUMBER ':' tUNUMBER ':' unsigned_seconds tMERIDIAN
         | iso_8601_time

   If accepted as time, return STATE_ACCEPT, or if its value is incorrect,
   return STATE_ABORT, otherwise, return STATE_STOP.  */

static int
parse_time (YYSTYPE *lvalp, parser_control *pc)
{
  char const *p0 = pc->input;
  YYSTYPE val0 = *lvalp;

  PARSING_START ("time", p0);

  int token = yylex (lvalp, pc);
  if (token != tUNUMBER)
    {
      pc->input = p0;
      *lvalp = val0;

      PARSING_STOP ("time");
      return STATE_STOP;
    }

  YYSTYPE val1 = *lvalp;
  token = yylex (lvalp, pc);
  if (token == tMERIDIAN)  /* tUNUMBER tMERIDIAN */
    {
      set_hhmmss (pc, val1.textintval.value, 0, 0, 0);
      pc->meridian = lvalp->intval;

      PARSING_ACCEPT ("time", "tUNUMBER tMERIDIAN");
      return STATE_ACCEPT;
    }
  else if (token == ':')
    {
      token = yylex (lvalp, pc);
      if (token == tUNUMBER)
        {
          YYSTYPE val3 = *lvalp;
          token = yylex (lvalp, pc);
          if (token == tMERIDIAN)  /* tUNUMBER ':' tUNUMBER tMERIDIAN */
            {
              set_hhmmss (
                pc, val1.textintval.value, val3.textintval.value, 0, 0);
              pc->meridian = lvalp->intval;

              PARSING_ACCEPT ("time", "tUNUMBER ':' tUNUMBER tMERIDIAN");
              return STATE_ACCEPT;
            }
          else if (token == ':')
            {
              YYSTYPE val5;
              switch (parse_seconds (lvalp, pc, false))
                {
                case STATE_ACCEPT:
                  val5 = *lvalp;
                  token = yylex (lvalp, pc);
                  if (token == tMERIDIAN) /* tUNUMBER ':' tUNUMBER ':'
                                             unsigned_seconds tMERIDIAN */
                    {
                      set_hhmmss (
                        pc, val1.textintval.value, val3.textintval.value,
                        val5.timespec.seconds, val5.timespec.nsec);
                      pc->meridian = lvalp->intval;

                      PARSING_ACCEPT ("time",
                                      "tUNUMBER ':' tUNUMBER ':' " \
                                      "unsigned_seconds tMERIDIAN");
                      return STATE_ACCEPT;
                    }
                  break;
                case STATE_ABORT:
                  return STATE_ABORT;
                }
            }
        }
    }

  pc->input = p0;
  *lvalp = val0;

  /* iso_8601_time */
  int state = parse_iso_8601_time (lvalp, pc);
  if (! next_parsing (state, "time", "iso_8601_time", NULL))
    return state;

  PARSING_STOP ("time");
  return STATE_STOP;
}


/* Parse the leading string as local_zone.

   local_zone := tLOCAL_ZONE
               | tLOCAL_ZONE tDST

   If accepted as local_zone, return STATE_ACCEPT, or if its value is
   incorrect, return STATE_ABORT, otherwise, return STATE_STOP.  */

static int
parse_local_zone (YYSTYPE *lvalp, parser_control *pc)
{
  char const *p0 = pc->input;
  YYSTYPE val0 = *lvalp;

  PARSING_START ("local_zone", p0);

  /* Local zone strings affect only the DST setting, and take effect
     only if the current TZ setting is relevant.

     Example 1:
     'EEST' is parsed as tLOCAL_ZONE, as it relates to the effective TZ:
          TZ='Europe/Helsinki' date -d '2016-06-30 EEST'

     Example 2:
     'EEST' is parsed as tDAYZONE:
          TZ='Asia/Tokyo' date -d '2016-06-30 EEST'

     This is implemented by probing the next three calendar quarters
     of the effective timezone and looking for DST changes -
     if found, the timezone name (EEST) is inserted into
     the lexical lookup table with type tLOCAL_ZONE.
     (Search for 'quarter' comment in  'parse_datetime2'.)
  */
  int token = yylex (lvalp, pc);
  if (token == tLOCAL_ZONE)
    {
      char const *p1 = pc->input;
      YYSTYPE val1 = *lvalp;
      token = yylex (lvalp, pc);
      if (token != tDST)  /* tLOCAL_ZONE */
        {
          pc->local_isdst = val1.intval;
          pc->input = p1;

          PARSING_ACCEPT ("local_zone", "tLOCAL_ZONE");
          return STATE_ACCEPT;
        }
      else  /* tLOCAL_ZONE tDST */
        {
          pc->local_isdst = 1;
          pc->dsts_seen++;

          PARSING_ACCEPT ("local_zone", "tLOCAL_ZONE tDST");
          return STATE_ACCEPT;
        }
    }

  pc->input = p0;
  *lvalp = val0;

  PARSING_STOP ("local_zone");
  return STATE_STOP;
}

static int parse_relunit_snumber (YYSTYPE *lvalp, parser_control *pc);

/* Parse the leading string as zone.

   zone := tZONE relunit_snumber
         | tZONE tSNUMBER o_colon_minutes
         | tZONE
         | tZONE tDST
         | 'T'
         | 'T' relunit_snumber
         | tDAYZONE

   If accepted as zone, return STATE_ACCEPT, or if its value is incorrect,
   return STATE_ABORT, otherwise, return STATE_STOP.  */

static int
parse_zone (YYSTYPE *lvalp, parser_control *pc)
{
  char const *p0 = pc->input;
  YYSTYPE val0 = *lvalp;

  PARSING_START ("zone", p0);

  int token = yylex (lvalp, pc);
  if (token == tZONE)
    {
      char const *p1 = pc->input;
      YYSTYPE val1;
      switch (parse_relunit_snumber (lvalp, pc))
        {
        case STATE_ACCEPT:  /* tZONE relunit_snumber */
          pc->time_zone = val1.intval;
          if (apply_relative_time (pc, lvalp->rel, 1))
            {
              PARSING_ACCEPT ("zone", "tZONE relunit_snumber");
              return STATE_ACCEPT;
            }
        case STATE_ABORT:
          PARSING_ABORT ("zone", "tZONE relunit_snumber");
          return STATE_ABORT;
        }

      val1 = *lvalp;
      token = yylex (lvalp, pc);
      if (token == tSNUMBER)
        {
          YYSTYPE val2 = *lvalp;
          switch (parse_o_colon_minutes (lvalp, pc))
            {
            case STATE_ACCEPT:  /* tZONE tSNUMBER o_colon_minutes */
              if (time_zone_hhmm (pc, val2.textintval, lvalp->intval)
                  && ! INT_ADD_WRAPV (
                         pc->time_zone, val1.intval, &pc->time_zone))
                {
                  PARSING_ACCEPT ("zone", "tZONE tSNUMBER o_colon_minutes");
                  return STATE_ACCEPT;
                }
            case STATE_ABORT:
              PARSING_ABORT ("zone", "tZONE tSNUMBER o_colon_minutes");
              return STATE_ABORT;
          }
        }
      else if (token != tDST)  /* tZONE */
        {
          pc->time_zone = val1.intval;
          pc->input = p1;

          PARSING_ACCEPT ("zone", "tZONE");
          return STATE_ACCEPT;
        }
      else  /* tZONE tDST */
        {
          pc->time_zone = val1.intval + 60 * 60;

          PARSING_ACCEPT ("zone", "tZONE tDST");
          return STATE_ACCEPT;
        }
    }
  else if (token == 'T')
    {
      /* Note 'T' is a special case, as it is used as the separator in ISO
         8601 date and time of day representation.  */
      pc->time_zone = -HOUR (7);

      switch (parse_relunit_snumber (lvalp, pc))
        {
        case STATE_ACCEPT:  /* 'T' relunit_snumber */
          if (apply_relative_time (pc, lvalp->rel, 1))
            {
              PARSING_ACCEPT ("zone", "'T' relunit_snumber");
              return STATE_ACCEPT;
            }

          PARSING_ABORT ("zone", "'T' relunit_snumber");
        case STATE_ABORT:
          return STATE_ABORT;
        }

      /* 'T' */
      PARSING_ACCEPT ("zone", "'T'");
      return STATE_ACCEPT;
    }
  else if (token == tDAYZONE)  /* tDAYZONE */
    {
      pc->time_zone = lvalp->intval + 60 * 60;

      PARSING_ACCEPT ("zone", "tDAYZONE");
      return STATE_ACCEPT;
    }

  pc->input = p0;
  *lvalp = val0;

  PARSING_STOP ("zone");
  return STATE_STOP;
}

/* Parse the leading string as day.

   day := tDAY
        | tDAY ','
        | tORDINAL tDAY
        | tUNUMBER tDAY

   If accepted as day, return STATE_ACCEPT, or if its value is incorrect,
   return STATE_ABORT, otherwise, return STATE_STOP.  */

static int
parse_day (YYSTYPE *lvalp, parser_control *pc)
{
  char const *p0 = pc->input;
  YYSTYPE val0 = *lvalp;

  PARSING_START ("day", p0);

  int token = yylex (lvalp, pc);
  if (token == tDAY)
    {
      char const *p1 = pc->input;
      pc->day_ordinal = 0;
      pc->day_number = lvalp->intval;
      token = yylex (lvalp, pc);
      if (token != ',')  /* tDAY */
        {
          pc->input = p1;

          PARSING_ACCEPT ("day", "tDAY");
          return STATE_ACCEPT;
        }

      /* tDAY ',' */
      PARSING_ACCEPT ("day", "tDAY ','");
      return STATE_ACCEPT;
    }
  else if (token == tORDINAL)
    {
      YYSTYPE val1 = *lvalp;
      token = yylex (lvalp, pc);
      if (token == tDAY)  /* tORDINAL tDAY */
        {
          pc->day_ordinal = val1.intval;
          pc->day_number = lvalp->intval;

          PARSING_ACCEPT ("day", "tORDINAL tDAY");
          return STATE_ACCEPT;
        }
    }
  else if (token == tUNUMBER)
    {
      YYSTYPE val1 = *lvalp;
      token = yylex (lvalp, pc);
      if (token == tDAY)  /* tUNUMBER tDAY */
        {
          pc->day_ordinal = val1.textintval.value;
          pc->day_number = lvalp->intval;

          PARSING_ACCEPT ("day", "tUNUMBER tDAY");
          return STATE_ACCEPT;
        }
    }

  pc->input = p0;
  *lvalp = val0;

  PARSING_STOP ("day");
  return STATE_STOP;
}

/* Parse the leading string as iso_8601_date.

   iso_8601_date := tUNUMBER tSNUMBER tSNUMBER

   If accepted as iso_8601_date, return STATE_ACCEPT, or if its value is
   incorrect, return STATE_ABORT, otherwise, return STATE_STOP.  */

static int
parse_iso_8601_date (YYSTYPE *lvalp, parser_control *pc)
{
  char const *p0 = pc->input;
  YYSTYPE val0 = *lvalp;

  PARSING_START ("iso_8601_date", p0);

  int token = yylex (lvalp, pc);
  if (token == tUNUMBER)
    {
      YYSTYPE val1 = *lvalp;
      token = yylex (lvalp, pc);
      if (token == tSNUMBER)
        {
          YYSTYPE val2 = *lvalp;
          token = yylex (lvalp, pc);
          if (token == tSNUMBER)  /* tUNUMBER tSNUMBER tSNUMBER */
            {
              /* ISO 8601 format.  YYYY-MM-DD.  */
              pc->year = val1.textintval;
              if (IMAX_SUBTRACT_WRAPV (0, val2.textintval.value, &pc->month)
                  || IMAX_SUBTRACT_WRAPV (
                        0, lvalp->textintval.value, &pc->day))
                {
                  PARSING_ABORT ("iso_8601_date",
                                 "tUNUMBER tSNUMBER tSNUMBER");
                  return STATE_ABORT;
                }
              pc->year_seen = true;

              PARSING_ACCEPT ("iso_8601_date", "tUNUMBER tSNUMBER tSNUMBER");
              return STATE_ACCEPT;
            }
        }
    }

  pc->input = p0;
  *lvalp = val0;

  PARSING_STOP ("iso_8601_date");
  return STATE_STOP;
}

/* Parse the leading string as date.

   date := tUNUMBER '/' tUNUMBER
         | tUNUMBER '/' tUNUMBER '/' tUNUMBER
         | tUNUMBER tMONTH tSNUMBER
         | tUNUMBER tMONTH
         | tUNUMBER tMONTH tUNUMBER
         | tMONTH tSNUMBER tSNUMBER
         | tMONTH tUNUMBER
         | tMONTH tUNUMBER ',' tUNUMBER
         | iso_8601_date

   If accepted as date, return STATE_ACCEPT, or if its value is incorrect,
   return STATE_ABORT, otherwise, return STATE_STOP.  */

static int
parse_date (YYSTYPE *lvalp, parser_control *pc)
{
  char const *p0 = pc->input;
  YYSTYPE val0 = *lvalp;

  PARSING_START ("date", p0);

  int token = yylex (lvalp, pc);
  if (token == tUNUMBER)
    {
      YYSTYPE val1 = *lvalp;
      token = yylex (lvalp, pc);
      if (token == '/')
        {
          token = yylex (lvalp, pc);
          if (token == tUNUMBER)
            {
              char const *p3 = pc->input;
              YYSTYPE val3 = *lvalp;
              token = yylex (lvalp, pc);
              if (token != '/')  /* tUNUMBER '/' tUNUMBER */
                {
                  pc->month = val1.textintval.value;
                  pc->day = val3.textintval.value;
                  pc->input = p3;

                  PARSING_ACCEPT ("date", "tUNUMBER '/' tUNUMBER");
                  return STATE_ACCEPT;
                }
              else
                {
                  token = yylex (lvalp, pc);
                  if (token == tUNUMBER)
                    /* tUNUMBER '/' tUNUMBER '/' tUNUMBER */
                    {
                      /* Interpret as YYYY/MM/DD if the first value has 4
                         or more digits, otherwise as MM/DD/YY.
                         The goal in recognizing YYYY/MM/DD is solely to
                         support legacy machine-generated dates like those
                         in an RCS log listing.  If you want portability,
                         use the ISO 8601 format.  */
                      if (4 <= val1.textintval.digits)
                        {
                          pc->year = val1.textintval;
                          pc->month = val3.textintval.value;
                          pc->day = lvalp->textintval.value;
                        }
                      else
                        {
                          pc->month = val1.textintval.value;
                          pc->day = val3.textintval.value;
                          pc->year = lvalp->textintval;
                        }
                      pc->year_seen = true;

                      PARSING_ACCEPT ("date",
                        "tUNUMBER '/' tUNUMBER '/' tUNUMBER");
                      return STATE_ACCEPT;
                    }
                }
            }
        }
      else if (token == tMONTH)
        {
          char const *p2 = pc->input;
          YYSTYPE val2 = *lvalp;
          token = yylex (lvalp, pc);
          if (token == tSNUMBER)  /* tUNUMBER tMONTH tSNUMBER */
            {
              /* e.g. 17-JUN-1992.  */
              pc->day = val1.textintval.value;
              pc->month = val2.intval;
              if (IMAX_SUBTRACT_WRAPV (
                    0, lvalp->textintval.value, &pc->year.value))
                {
                  PARSING_ABORT ("date", "tUNUMBER tMONTH tSNUMBER");
                  return STATE_ABORT;
                }
              pc->year.digits = lvalp->textintval.digits;
              pc->year_seen = true;

              PARSING_ACCEPT ("date", "tUNUMBER tMONTH tSNUMBER");
              return STATE_ACCEPT;
            }
          else if (token != tUNUMBER)  /* tUNUMBER tMONTH */
            {
              pc->day = val1.textintval.value;
              pc->month = val2.intval;
              pc->input = p2;

              PARSING_ACCEPT ("date", "tUNUMBER tMONTH");
              return STATE_ACCEPT;
            }
          else  /* tUNUMBER tMONTH tUNUMBER */
            {
              pc->day = val1.textintval.value;
              pc->month = val2.intval;
              pc->year = lvalp->textintval;
              pc->year_seen = true;

              PARSING_ACCEPT ("date", "tUNUMBER tMONTH tUNUMBER");
              return STATE_ACCEPT;
            }
        }
    }
  else if (token == tMONTH)
    {
      YYSTYPE val1 = *lvalp;
      token = yylex (lvalp, pc);
      if (token == tSNUMBER)
        {
          YYSTYPE val2 = *lvalp;
          token = yylex (lvalp, pc);
          if (token == tSNUMBER)  /* tMONTH tSNUMBER tSNUMBER */
            {
              /* e.g. JUN-17-1992.  */
              pc->month = val1.intval;
              if (IMAX_SUBTRACT_WRAPV (0, val2.textintval.value, &pc->day)
                  || IMAX_SUBTRACT_WRAPV (
                       0, lvalp->textintval.value, &pc->year.value))
                {
                  PARSING_ABORT ("date", "tMONTH tSNUMBER tSNUMBER");
                  return STATE_ABORT;
                }
              pc->year.digits = lvalp->textintval.digits;
              pc->year_seen = true;

              PARSING_ACCEPT ("date", "tMONTH tSNUMBER tSNUMBER");
              return STATE_ACCEPT;
           }
        }
      else if (token == tUNUMBER)
        {
          char const *p2 = pc->input;
          YYSTYPE val2 = *lvalp;
          token = yylex (lvalp, pc);
          if (token != ',')  /* tMONTH tUNUMBER */
            {
              pc->month = val1.intval;
              pc->day = val2.textintval.value;
              pc->input = p2;

              PARSING_ACCEPT ("date", "tMONTH tUNUMBER");
              return STATE_ACCEPT;
            }
          else
            {
              token = yylex (lvalp, pc);
              if (token == tUNUMBER)  /* tMONTH tUNUMBER ',' tUNUMBER */
                {
                  pc->month = val1.intval;
                  pc->day = val2.textintval.value;
                  pc->year = lvalp->textintval;
                  pc->year_seen = true;

                  PARSING_ACCEPT ("date", "tMONTH tUNUMBER ',' tUNUMBER");
                  return STATE_ACCEPT;
                }
            }
        }
    }

  pc->input = p0;
  *lvalp = val0;

  /* iso_8601_date */
  int state = parse_iso_8601_date (lvalp, pc);
  if (! next_parsing (state, "date", "iso_8601_date", NULL))
    return state;

  PARSING_STOP ("date");
  return STATE_STOP;
}

/* Parse the leading string as relunit_snumber.

   relunit := tSNUMBER tYEAR_UNIT
            | tSNUMBER tMONTH_UNIT
            | tSNUMBER tDAY_UNIT
            | tSNUMBER tHOUR_UNIT
            | tSNUMBER tMINUTE_UNIT
            | tSNUMBER tSEC_UNIT

   If accepted as relunit_snumber, return STATE_ACCEPT, or if its value is
   incorrect, return STATE_ABORT, otherwise, return STATE_STOP.  */

static int
parse_relunit_snumber (YYSTYPE *lvalp, parser_control *pc)
{
  char const *p0 = pc->input;
  YYSTYPE val0 = *lvalp;

  PARSING_START ("relunit_snumber", p0);

  int token = yylex (lvalp, pc);
  if (token != tSNUMBER)
    {
      pc->input = p0;
      *lvalp = val0;

      PARSING_STOP ("relunit_snumber");
      return STATE_STOP;
    }

  intmax_t num_value = lvalp->textintval.value;
  intmax_t rel_value;
  int relunit_token = yylex (lvalp, pc);
  switch (relunit_token)
    {
    case tYEAR_UNIT:  /* tSNUMBER tYEAR_UNIT */
      lvalp->rel = RELATIVE_TIME_0;
      lvalp->rel.year = num_value;
      break;
    case tMONTH_UNIT:  /* tSNUMBER tMONTH_UNIT */
      lvalp->rel = RELATIVE_TIME_0;
      lvalp->rel.month = num_value;
      break;
    case tDAY_UNIT:  /* tSNUMBER tDAY_UNIT */
      rel_value = lvalp->intval;
      lvalp->rel = RELATIVE_TIME_0;
      if (IMAX_MULTIPLY_WRAPV (num_value, rel_value, &(lvalp->rel).day))
        {
          PARSING_ABORT_RELUNIT ("relunit_snumber", tSNUMBER, tDAY_UNIT);
          return STATE_ABORT;
        }
      break;
    case tHOUR_UNIT:  /* tSNUMBER tHOUR_UNIT */
      lvalp->rel = RELATIVE_TIME_0;
      lvalp->rel.hour = num_value;
      break;
    case tMINUTE_UNIT:  /* tSNUMBER tMINUTE_UNIT */
      lvalp->rel = RELATIVE_TIME_0;
      lvalp->rel.minutes = num_value;
      break;
    case tSEC_UNIT:  /* tSNUMBER tSEC_UNIT */
      lvalp->rel = RELATIVE_TIME_0;
      lvalp->rel.seconds = num_value;
      break;
    default:
      pc->input = p0;
      *lvalp = val0;

      PARSING_STOP ("relunit_snumber");
      return STATE_STOP;
    }

  PARSING_ACCEPT_RELUNIT ("relunit_snumber", tSNUMBER, relunit_token);
  return STATE_ACCEPT;
}

/* Parse the leading string as relunit.

   relunit := relunit_snumber
            | tORDINAL tYEAR_UNIT
            | tUNUMBER tYEAR_UNIT
            | tYEAR_UNIT
            | tORDINAL tMONTH_UNIT
            | tUNUMBER tMONTH_UNIT
            | tMONTH_UNIT
            | tORDINAL tDAY_UNIT
            | tUNUMBER tDAY_UNIT
            | tDAY_UNIT
            | tORDINAL tHOUR_UNIT
            | tUNUMBER tHOUR_UNIT
            | tHOUR_UNIT
            | tORDINAL tMINUTE_UNIT
            | tUNUMBER tMINUTE_UNIT
            | tMINUTE_UNIT
            | tORDINAL tSEC_UNIT
            | tUNUMBER tSEC_UNIT
            | tSDECIMAL_NUMBER tSEC_UNIT
            | tUDECIMAL_NUMBER tSEC_UNIT
            | tSEC_UNIT

   If accepted as relunit, return STATE_ACCEPT, or if its value is incorrect,
   return STATE_ABORT, otherwise, return STATE_STOP.  */

static int
parse_relunit (YYSTYPE *lvalp, parser_control *pc)
{
  char const *p0 = pc->input;

  PARSING_START ("relunit", p0);
  YYSTYPE val0 = *lvalp;

  /* relunit_snumber */
  int state = parse_relunit_snumber (lvalp, pc);
  if (! next_parsing (state, "relunit", "relunit_snumber", NULL))
    return state;

  intmax_t num_value = 1;
  int num_token = -1;
  int relunit_token;
  int token = yylex (lvalp, pc);
  if (token == tORDINAL)
    {
      num_token = token;
      num_value = lvalp->intval;
      relunit_token = yylex (lvalp, pc);
    }
  else if (token == tUNUMBER)
    {
      num_token = token;
      num_value = lvalp->textintval.value;
      relunit_token = yylex (lvalp, pc);
    }
  else if (token == tSDECIMAL_NUMBER || token == tUDECIMAL_NUMBER)
    {
      YYSTYPE val1 = *lvalp;
      relunit_token = yylex (lvalp, pc);
      if (relunit_token == tSEC_UNIT)
        /* [ tSDECIMAL_NUMBER | tUDECIMAL_NUMBER ] tSEC_UNIT */
        {
          lvalp->rel = RELATIVE_TIME_0;
          lvalp->rel.seconds = val1.timespec.seconds;
          lvalp->rel.ns = val1.timespec.nsec;

          PARSING_ACCEPT_RELUNIT ("relunit", token, tSEC_UNIT);
          return STATE_ACCEPT;
        }

      pc->input = p0;
      *lvalp = val0;

      PARSING_STOP ("relunit");
      return STATE_STOP;
    }
  else
    relunit_token = token;

  intmax_t rel_value;
  switch (relunit_token)
    {
    case tYEAR_UNIT:  /* [ tORDINAL | tUNUMBER ] tYEAR_UNIT */
      lvalp->rel = RELATIVE_TIME_0;
      lvalp->rel.year = num_value;
      break;
    case tMONTH_UNIT:  /* [ tORDINAL | tUNUMBER ] tMONTH_UNIT */
      lvalp->rel = RELATIVE_TIME_0;
      lvalp->rel.month = num_value;
      break;
    case tDAY_UNIT:  /* [ tORDINAL | tUNUMBER ] tDAY_UNIT */
      rel_value = lvalp->intval;
      lvalp->rel = RELATIVE_TIME_0;
      if (IMAX_MULTIPLY_WRAPV (num_value, rel_value, &(lvalp->rel).day))
        {
          PARSING_ABORT_RELUNIT ("relunit", num_token, tDAY_UNIT);
          return STATE_ABORT;
        }
      break;
    case tHOUR_UNIT:  /* [ tORDINAL | tUNUMBER ] tHOUR_UNIT */
      lvalp->rel = RELATIVE_TIME_0;
      lvalp->rel.hour = num_value;
      break;
    case tMINUTE_UNIT:  /* [ tORDINAL | tUNUMBER ] tMINUTE_UNIT */
      lvalp->rel = RELATIVE_TIME_0;
      lvalp->rel.minutes = num_value;
      break;
    case tSEC_UNIT:  /* [ tORDINAL | tUNUMBER ] tSEC_UNIT */
      lvalp->rel = RELATIVE_TIME_0;
      lvalp->rel.seconds = num_value;
      break;
    default:
      pc->input = p0;
      *lvalp = val0;

      PARSING_STOP ("relunit");
      return STATE_STOP;
    }

  PARSING_ACCEPT_RELUNIT ("relunit", num_token, relunit_token);
  return STATE_ACCEPT;
}

/* Parse the leading string as dayshift.

   dayshift := tDAY_SHIFT

   If accepted as dayshift, return STATE_ACCEPT, or if its value is incorrect,
   return STATE_ABORT, otherwise, return STATE_STOP.  */

static int
parse_dayshift (YYSTYPE *lvalp, parser_control *pc)
{
  char const *p0 = pc->input;
  YYSTYPE val0 = *lvalp;

  PARSING_START ("dayshift", p0);

  int token = yylex (lvalp, pc);
  if (token == tDAY_SHIFT)  /* tDAY_SHIFT */
    {
      intmax_t rel_value = lvalp->intval;
      lvalp->rel = RELATIVE_TIME_0;
      lvalp->rel.day = rel_value;

      PARSING_ACCEPT ("dayshift", "tDAY_SHIFT");
      return STATE_ACCEPT;
    }

  pc->input = p0;
  *lvalp = val0;

  PARSING_STOP ("dayshift");
  return STATE_STOP;
}

/* Parse the leading string as rel.

   rel := relunit tAGO
        | relunit
        | dayshift

   If accepted as rel, return STATE_ACCEPT, or if its value is incorrect,
   return STATE_ABORT, otherwise, return STATE_STOP.  */

static int
parse_rel (YYSTYPE *lvalp, parser_control *pc)
{
  PARSING_START ("rel", pc->input);

  char const *p1;
  YYSTYPE val1;
  int token;

  switch (parse_relunit (lvalp, pc))
    {
    case STATE_ACCEPT:
      p1 = pc->input;
      val1 = *lvalp;
      token = yylex (lvalp, pc);
      if (token == tAGO)  /* relunit tAGO */
        {
          if (apply_relative_time (pc, val1.rel, lvalp->intval))
            {
              PARSING_ACCEPT ("parse_rel", "relunit tAGO");
              return STATE_ACCEPT;
            }

            PARSING_ABORT ("parse_rel", "relunit tAGO");
        }
      else  /* relunit */
        {
          if (apply_relative_time (pc, val1.rel, 1))
            {
              pc->input = p1;
              *lvalp = val1;

              PARSING_ACCEPT ("parse_rel", "relunit");
              return STATE_ACCEPT;
            }

          PARSING_ABORT ("parse_rel", "relunit");
        }
    case STATE_ABORT:
      return STATE_ABORT;
    }

  /* dayshift */
  switch (parse_dayshift (lvalp, pc))
    {
    case STATE_ACCEPT:
      if (apply_relative_time (pc, lvalp->rel, 1))
        {
          PARSING_ACCEPT ("parse_rel", "dayshift");
          return STATE_ACCEPT;
        }

      PARSING_ABORT ("parse_rel", "dayshift");
    case STATE_ABORT:
      return STATE_ABORT;
    }

  PARSING_STOP ("rel");
  return STATE_STOP;
}

/* Extract into *PC any date and time info from a string of digits
   of the form e.g., YYYYMMDD, YYMMDD, HHMM, HH (and sometimes YYY,
   YYYY, ...).  */
static void
digits_to_date_time (parser_control *pc, textint text_int)
{
  if (pc->dates_seen && ! pc->year.digits
      && ! pc->rels_seen && (pc->times_seen || 2 < text_int.digits))
    {
      pc->year_seen = true;
      pc->year = text_int;

      PARSING_ACCEPT ("number", "\"YYYY\"");
    }
  else
    {
      if (4 < text_int.digits)
        {
          pc->dates_seen++;
          pc->day = text_int.value % 100;
          pc->month = (text_int.value / 100) % 100;
          pc->year.value = text_int.value / 10000;
          pc->year.digits = text_int.digits - 4;
          pc->year_seen = true;

          PARSING_ACCEPT ("number", "\"YYYYMMDD\"");
        }
      else
        {
          pc->times_seen++;
          if (text_int.digits <= 2)
            {
              pc->hour = text_int.value;
              pc->minutes = 0;

              PARSING_ACCEPT ("number", "\"HH\"");
            }
          else
            {
              pc->hour = text_int.value / 100;
              pc->minutes = text_int.value % 100;

              PARSING_ACCEPT ("number", "\"HHMM\"");
            }
          pc->seconds = 0;
          pc->nsec = 0;
          pc->meridian = MER24;
        }
    }
}

/* Parse the leading string as number.

   number := tUNUMBER

   If accepted as number, return STATE_ACCEPT, or if its value is incorrect,
   return STATE_ABORT, otherwise, return STATE_STOP.  */

static int
parse_number (YYSTYPE *lvalp, parser_control *pc)
{
  char const *p0 = pc->input;
  YYSTYPE val0 = *lvalp;

  PARSING_START ("number", p0);

  int token = yylex (lvalp, pc);
  if (token == tUNUMBER)  /* tUNUMBER */
    {
      digits_to_date_time (pc, lvalp->textintval);

      PARSING_ACCEPT ("number", "tUNUMBER");
      return STATE_ACCEPT;
    }

  pc->input = p0;
  *lvalp = val0;

  PARSING_STOP ("number");
  return STATE_STOP;
}

/* Parse the leading string as hybrid.

   hybrid := tUNUMBER relunit_snumber

   If accepted as hybrid, return STATE_ACCEPT, or if its value is incorrect,
   return STATE_ABORT, otherwise, return STATE_STOP.  */

static int
parse_hybrid (YYSTYPE *lvalp, parser_control *pc)
{
  char const *p0 = pc->input;
  YYSTYPE val0 = *lvalp;

  PARSING_START ("hybrid", p0);

  int token = yylex (lvalp, pc);
  if (token == tUNUMBER)
    {
      YYSTYPE val1 = *lvalp;

      /* tUNUMBER relunit_snumber */
      switch (parse_relunit_snumber (lvalp, pc))
        {
        case STATE_ACCEPT:
          /* Hybrid all-digit and relative offset, so that we accept e.g.,
             "YYYYMMDD +N days" as well as "YYYYMMDD N days".  */
          digits_to_date_time (pc, val1.textintval);
          if (apply_relative_time (pc, lvalp->rel, 1))
            {
              PARSING_ACCEPT ("hybrid", "tUNUMBER relunit_snumber");
              return STATE_ACCEPT;
            }

          PARSING_ABORT ("hybrid", "tUNUMBER relunit_snumber");
        case STATE_ABORT:
          return STATE_ABORT;
        }
    }

  pc->input = p0;
  *lvalp = val0;

  PARSING_STOP ("hybrid");
  return STATE_STOP;
}

/* Parse the leading string as item.

   item := datetime
         | date
         | time
         | local_zone
         | zone
         | day
         | rel
         | number
         | hybrid
         | 'J'

   If accepted as item, return STATE_ACCEPT, or if its value is incorrect,
   return STATE_ABORT, otherwise, return STATE_STOP.  */

static int
parse_item (YYSTYPE *lvalp, parser_control *pc)
{
  char const *p0 = pc->input;
  YYSTYPE val0 = *lvalp;

  PARSING_START ("item", pc->input);

  /* datetime */
  switch (parse_datetime (lvalp, pc))
    {
     case STATE_ACCEPT:
       pc->times_seen++;
       pc->dates_seen++;
       PARSING_ACCEPT ("item", "datetime");
       return STATE_ACCEPT;
     case STATE_ABORT:
       return STATE_ABORT;
     }

  /* date */
  int state = parse_date (lvalp, pc);
  if (! next_parsing (state, "item", "date", &pc->dates_seen))
    return state;

  /* hybrid */
  state = parse_hybrid (lvalp, pc);
  if (! next_parsing (state, "item", "hybrid", NULL))
    return state;

  /* time */
  state = parse_time (lvalp, pc);
  if (! next_parsing (state, "item", "time", &pc->times_seen))
    return state;

  /* local_zone */
  state = parse_local_zone (lvalp, pc);
  if (! next_parsing (state, "item", "local_zone", &pc->local_zones_seen))
    return state;

  /* zone */
  state = parse_zone (lvalp, pc);
  if (! next_parsing (state, "item", "zone", &pc->zones_seen))
    return state;

  /* day */
  state = parse_day (lvalp, pc);
  if (! next_parsing (state, "item", "day", &pc->days_seen))
    return state;

  /* rel */
  state = parse_rel (lvalp, pc);
  if (! next_parsing (state, "item", "rel", NULL))
    return state;

  /* number */
  state = parse_number (lvalp, pc);
  if (! next_parsing (state, "item", "number", NULL))
    return state;

  int token = yylex (lvalp, pc);
  if (token == 'J')  /* 'J' */
    {
      pc->J_zones_seen++;

      PARSING_ACCEPT ("item", "'J'");
      return STATE_ACCEPT;
    }

  pc->input = p0;
  *lvalp = val0;

  PARSING_STOP ("item");
  return STATE_STOP;
}

#ifdef TEST
/* The count of nesting items.  */
static unsigned int nesting_items = 0;

/* Output the state of parsing an expression in the nest of items.  */
# define PARSING_ITEMS(state,nest,symbol,expr) \
           (parsing_output \
            ? print_parsing_state (state, nest, symbol, expr) : 0)
# define PARSING_START_ITEMS(nest,symbol,expr) \
           PARSING_ITEMS (STATE_START, nest, symbol, expr)
# define PARSING_ACCEPT_ITEMS(nest,symbol,expr) \
           PARSING_ITEMS (STATE_ACCEPT, nest, symbol, expr)
# define PARSING_STOP_ITEMS(nest,symbol) \
           PARSING_ITEMS (STATE_STOP, nest, symbol, NULL)
#else
# define PARSING_START_ITEMS(nest,symbol,expr)
# define PARSING_ACCEPT_ITEMS(nest,symbol,expr)
# define PARSING_STOP_ITEMS(nest,symbol)
#endif

/* Parse the leading string as items.

   items := empty | item items

   If accepted as items, return STATE_ACCEPT, or if its value is incorrect,
   return STATE_ABORT, otherwise, return STATE_STOP.  */

static int
parse_items (YYSTYPE *lvalp, parser_control *pc)
{
  char const *p0 = pc->input;
  YYSTYPE val0 = *lvalp;

  PARSING_START_ITEMS (++nesting_items, "items", p0);

  int token = yylex (lvalp, pc);
  if (token == '\0')  /* empty */
    {
      PARSING_ACCEPT_ITEMS (nesting_items--, "items", "empty");
      return STATE_ACCEPT;
    }

  pc->input = p0;
  *lvalp = val0;

  int state;
  switch (parse_item (lvalp, pc))
    {
    case STATE_ACCEPT:
      state = parse_items (lvalp, pc);
      if (state == STATE_ACCEPT)  /* item items */
        {
          PARSING_ACCEPT_ITEMS (nesting_items--, "items", "item items");
          return STATE_ACCEPT;
        }
      else if (state == STATE_STOP)
        break;
    case STATE_ABORT:
      return STATE_ABORT;
    }

  PARSING_STOP_ITEMS (nesting_items--, "items");
  return STATE_STOP;
}

/* Parse the leading string as spec.

   spec := timespec | items

   If accepted as spec, return STATE_ACCEPT, or if its value is incorrect,
   return STATE_ABORT, otherwise, return STATE_STOP.  */

static int
parse_spec (YYSTYPE *lvalp, parser_control *pc)
{
  PARSING_START ("spec", pc->input);

  /* timespec */
  int state = parse_timespec (lvalp, pc);
  if (! next_parsing (state, "spec", "timespec", NULL))
    return state;

  /* items */
  state = parse_items (lvalp, pc);
  if (! next_parsing (state, "spec", "items", NULL))
    return state;

  PARSING_STOP ("spec");
  return STATE_STOP;
}

/* Parse a date and time string. */

static bool
parse (parser_control *pc)
{
  YYSTYPE lval;
  int ret = parse_spec (&lval, pc);
  if (ret == STATE_ACCEPT)  /* spec */
    return true;

  return false;
}

/* Populate PC's local time zone table with information from TM.  */

static void
populate_local_time_zone_table (parser_control *pc, TM const *lct)
{
  bool first_entry_exists = !!pc->local_time_zone_table[0].name;

  /* The table entry to be filled in.  There are only two, so this is
     the first entry if it is missing, the second entry otherwise.  */
  table *e = &pc->local_time_zone_table[first_entry_exists];

  e->type = tLOCAL_ZONE;
  e->value = lct->tm_isdst;

  const struct tm *tmp;

#ifdef USE_TM_GLIBC
  tmp = lct;
#else
  struct tm tm;

  tm.tm_year = lct->tm_year;
  tm.tm_mon = lct->tm_mon;
  tm.tm_mday = lct->tm_mday;
  tm.tm_wday = lct->tm_wday;
  tm.tm_hour = lct->tm_hour;
  tm.tm_min = lct->tm_min;
  tm.tm_sec = lct->tm_sec;
  tm.tm_isdst = lct->tm_isdst;
  tmp = &tm;
#endif

  char *tz_abbr = pc->tz_abbr[first_entry_exists];
  char const *zone = NULL;
  int size = strftime (tz_abbr, TIME_ZONE_BUFSIZE, "%Z", tmp);
  if (size)
    {
#ifndef USE_TM_GLIBC
      /* Remove the trailing characters enclosed in two parenthesis from
         the time zone name.  */
      if (tz_abbr[size - 1] == ')')
        {
          char *p = tz_abbr + size - 1;
          while (--p >= tz_abbr)
            {
              if (*p == '(')
                {
                  if (p > tz_abbr && isspace (*(p - 1)))
                    p--;
                  *p = '\0';
                  break;
                }
            }
        }

      /* Remove white-space characters from the time zone name.  */
      char *p = tz_abbr;
      do
        {
          size--;
          if (isspace (*p))
            {
              int i;
              for (i = 0; i < size; i++)
                p[i] = p[i + 1];
              p[i] = '\0';
            }
          else
            p++;
        }
      while (size >= 0);
#endif

      zone = pc->tz_abbr[first_entry_exists];
    }

  e->name = zone;
  e[1].name = NULL;
}

/* Parse a date/time string, storing the resulting parameters of time into
   *RESULT.  The string itself is pointed to by P which can be an incomplete
   or relative time specification.  Return true if successful.  */
bool
parseft (FT_PARSING *result, char const *p)
{
  FT_CHANGE ft_chg =
    (FT_CHANGE) { .date_set = false, .year = -1, .hour = -1, .minutes = -1,
                  .seconds = -1, .ns = -1, .day_number = -1, .tz_set = false,
                  .lctz_isdst = -1, .modflag = result->change.modflag };
  FT now;
  if (! currentft (&now))
    return false;

  intmax_t Start;
  int Start_ns;
  if (! ft2sec (&now, &Start, &Start_ns))
    return false;

  unsigned char c;
  while (c = *p, isspace (c))
    p++;

  /* Store a local copy prior to first "goto".  Without this, a prior use
     below of RELATIVE_TIME_0 on the RHS might translate to an assignment-
     to-temporary, which would trigger a -Wjump-misses-init warning.  */
  const relative_time rel_time_0 = RELATIVE_TIME_0;

  /* Never use the environment variable of a time zone ('TZ="XXX"').  */

  TM tmp;
  if (! localtimew (&Start, &tmp))
    return false;

  /* As documented, be careful to treat the empty string just like
     a date string of "0".  Without this, an empty string would be
     declared invalid when parsed during a DST transition.  */
  if (*p == '\0')
    p = "0";

  parser_control pc;
  pc.input = p;
  pc.year.value = 0;
  pc.year.digits = 0;
  pc.month = 0;
  pc.day = 0;
  pc.hour = 0;
  pc.minutes = 0;
  pc.seconds = 0;
  pc.nsec = 0;
  pc.time_zone = 0;

  pc.meridian = MER24;
  pc.rel = rel_time_0;
  pc.timespec_seen = false;
  pc.rels_seen = false;
  pc.dates_seen = 0;
  pc.days_seen = 0;
  pc.times_seen = 0;
  pc.J_zones_seen = 0;
  pc.local_zones_seen = 0;
  pc.dsts_seen = 0;
  pc.zones_seen = 0;
  pc.year_seen = false;

  pc.local_time_zone_table[0].name = NULL;
  populate_local_time_zone_table (&pc, &tmp);

  /* Probe the names used in the next three calendar quarters, looking
     for a tm_isdst different from the one we already have.  */
  for (int quarter = 1; quarter <= 3; quarter++)
    {
      intmax_t probe;
      if (IMAX_ADD_WRAPV (Start, quarter * (90 * 24 * 60 * 60), &probe))
        break;
      TM probe_tm;
      if (localtimew (&probe, &probe_tm)
          && (! pc.local_time_zone_table[0].name
              || probe_tm.tm_isdst != pc.local_time_zone_table[0].value))
        {
          populate_local_time_zone_table (&pc, &probe_tm);
          if (pc.local_time_zone_table[1].name)
            {
              if (! strcmp (pc.local_time_zone_table[0].name,
                            pc.local_time_zone_table[1].name))
                {
                  /* This locale uses the same abbreviation for standard and
                     daylight times.  So if we see that abbreviation, we don't
                     know whether it's daylight time.  */
                  pc.local_time_zone_table[0].value = -1;
                  pc.local_time_zone_table[1].name = NULL;
                }

              break;
            }
        }
    }

#ifndef USE_TM_GLIBC
  pc.ansi_cp = 0;

  char buff[6] = { '\0' };

  if (GetLocaleInfo (LOCALE_SYSTEM_DEFAULT,
        LOCALE_IDEFAULTANSICODEPAGE, (LPTSTR)buff, 6) > 0)
    pc.ansi_cp = atoi (buff);
#endif

  if (! parse (&pc))
    return false;

  if (! pc.timespec_seen)
    {
      if (1 < (pc.times_seen | pc.dates_seen | pc.days_seen | pc.dsts_seen
               | (pc.J_zones_seen + pc.local_zones_seen + pc.zones_seen))
          || pc.rel.year < INT_MIN || pc.rel.year > INT_MAX
          || pc.rel.month < INT_MIN || pc.rel.month > INT_MAX
          || pc.rel.day < INT_MIN || pc.rel.day > INT_MAX)
        return false;

      if (pc.dates_seen)
        {
          if (pc.year_seen)
            {
              int tm_year;
              if (! to_tm_year (pc.year, &tm_year))
                return false;
              ft_chg.year = tm_year + TM_YEAR_BASE;
            }

          int tm_mon;
          if (pc.month < INT_MIN || pc.month > INT_MAX
              || pc.day < INT_MIN || pc.day > INT_MAX
              || INT_ADD_WRAPV (pc.month, -1, &tm_mon)
              || INT_ADD_WRAPV (pc.day, 0, &ft_chg.day))
            return false;
          ft_chg.month = tm_mon + 1;
          ft_chg.date_set = true;
        }
      if (pc.times_seen)
        {
          int hour = to_hour (pc.hour, pc.meridian);
          if (hour < 0)
            return false;
          ft_chg.hour = hour;
          ft_chg.minutes = pc.minutes;
          ft_chg.seconds = pc.seconds;
          ft_chg.ns = pc.nsec;
        }

      if (pc.days_seen && ! pc.dates_seen)
        {
          ft_chg.day_number = pc.day_number;
          ft_chg.day_ordinal = pc.day_ordinal;
        }

      ft_chg.rel_set = pc.rels_seen;
      ft_chg.rel_year = pc.rel.year;
      ft_chg.rel_month = pc.rel.month;
      ft_chg.rel_day = pc.rel.day;
      ft_chg.rel_hour = pc.rel.hour;
      ft_chg.rel_minutes = pc.rel.minutes;
      ft_chg.rel_seconds = pc.rel.seconds;
      ft_chg.rel_ns = pc.rel.ns;
      ft_chg.tz_set = pc.zones_seen;

      if (pc.zones_seen)
        ft_chg.tz_utcoff = pc.time_zone;

      if (pc.local_zones_seen)
        ft_chg.lctz_isdst = pc.local_isdst;

      result->change = ft_chg;
    }
  else if (! sec2ft (pc.seconds, pc.nsec, &result->timespec.ft))
    return false;

  result->timespec_seen = pc.timespec_seen;

  return true;
}

#ifdef TEST
# include <unistd.h>

# include "cmdtmio.h"
# include "exit.h"

static void
usage (int status)
{
  printusage ("parseft", " STRING\n\
Parse STRING as the representation of date and time into parameters\n\
by which file time is changed. Display those values if parsing is\n\
completed and parameters are not duplicate, otherwise, nothing.\n\
\n\
Options:\n\
  -p   output the state of each parsing instead of values.\
", true, false, 0);
  exit (status);
}

int
main (int argc, char **argv)
{
  FT_PARSING result;
  FT_CHANGE *ft_chgp = &(result.change);
  int c;

  while ((c = getopt (argc, argv, ":p")) != -1)
    {
      switch (c)
        {
        case 'p':
          parsing_output = true;
          break;
        default:
          usage (EXIT_FAILURE);
        }
    }

  argc -= optind;
  argv += optind;

  if (argc <= 0)
    usage (EXIT_FAILURE);

  if (! parseft (&result, *argv))
    return EXIT_FAILURE;
  else if (parsing_output)
    return EXIT_SUCCESS;

  if (result.timespec_seen)  /* '@' SECONDS[.nnnnnnn] */
    {
      intmax_t seconds;
      int nsec;
      ft2sec (&result.timespec.ft, &seconds, &nsec);

      printelapse (false, seconds, nsec);
    }
  else  /* A variety of date and time string */
    {
      int *dates[] = { &ft_chgp->year, &ft_chgp->month, &ft_chgp->day };
      int *times[] = { &ft_chgp->hour, &ft_chgp->minutes, &ft_chgp->seconds };
      long int tz_utcoff = ft_chgp->tz_utcoff;

      /* Output a date and time in ISO 8601 format, instead of current time. */
      struct tm_fmt tm_fmt = (struct tm_fmt) { .iso8601 = true,
                                               .no_newline = true };
      struct tm_ptrs tm_ptrs =
        (struct tm_ptrs) { .dates = ft_chgp->month <= 0 ? NULL : dates,
                           .times = ft_chgp->hour < 0 ? NULL : times,
                           .ns = ft_chgp->ns < 0 ? NULL : &ft_chgp->ns };
      if (ft_chgp->day_number >= 0)
        {
          tm_fmt.weekday_name = true;
          tm_ptrs.weekday = &ft_chgp->day_number;
          if (ft_chgp->day_ordinal > 0)
            tm_ptrs.weekday_ordinal = &ft_chgp->day_ordinal;
        }
      if (ft_chgp->tz_set)
        tm_ptrs.utcoff = &tz_utcoff;
      else if (ft_chgp->hour < 0)  /* -0001-00-00 */
        tm_ptrs.dates = dates;

      printtm (&tm_fmt, &tm_ptrs);
      printisdst (ft_chgp->rel_set, ft_chgp->lctz_isdst);

      /* Output relative values, added to or subtracted from current time. */
      if (ft_chgp->rel_set)
        {
          int *rel_dates[] =
            { &ft_chgp->rel_year, &ft_chgp->rel_month, &ft_chgp->rel_day };
          intmax_t *rel_times[] = { &ft_chgp->rel_hour,
                                    &ft_chgp->rel_minutes,
                                    &ft_chgp->rel_seconds };
          tm_ptrs = (struct tm_ptrs) { .dates = rel_dates,
                                       .rel_times = rel_times,
                                       .ns = &ft_chgp->rel_ns };

          printreltm (false, &tm_ptrs);
        }
    }

  return EXIT_SUCCESS;
}
#endif
