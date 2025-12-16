CC=x86_64-w64-mingw32-gcc
CC_X86=i686-w64-mingw32-gcc
CFLAGS=-std=c11 -O2 -Wformat -Werror
GCC=gcc
GFLAGS=-std=gnu11 -O2 -Wformat -Werror

TOUCH_OBJS=argempty.o argmatch.o argnumint.o currentft.o errft.o ft2sec.o \
           getft.o imaxoverflow.o intoverflow.o localtime.o mktime.o \
           modifysec.o parseft.o posixtm.o sec2ft.o secoverflow.o setft.o

ADJUSTDAY_OBJS=adjusttm.o argempty.o argnumimax.o argnumint.o argreltm.o \
               error.o imaxoverflow.o intoverflow.o leapdays.o printtm.o \
               printusage.o weekday.o yeardays.o

GETFT_OBJS=argempty.o argnumint.o currentft.o errft.o ft2sec.o ft2val.o \
           imaxoverflow.o intoverflow.o modifysec.o printelapse.o \
           printusage.o secoverflow.o

CURRENTFT_OBJS=error.o ft2sec.o ft2val.o imaxoverflow.o modifysec.o \
               printelapse.o printusage.o secoverflow.o

LEAPDAYS_OBJS=argempty.o argnumint.o error.o imaxoverflow.o intoverflow.o \
              printusage.o

LOCALTIME_OBJS=argempty.o argnumimax.o argnumint.o argseconds.o error.o \
               imaxoverflow.o intoverflow.o printisdst.o printtm.o \
               printusage.o secoverflow.o yeardays.o

MKTIME_OBJS=argempty.o argisdst.o argmatch.o argnumimax.o argnumint.o \
            argreltm.o error.o imaxoverflow.o intoverflow.o printelapse.o \
            printisdst.o printtm.o printusage.o yeardays.o

MODIFYSEC_OBJS=argempty.o argnumimax.o argnumint.o argseconds.o currentft.o \
               error.o imaxoverflow.o intoverflow.o printelapse.o \
               printusage.o secoverflow.o

PARSEFT_OBJS=currentft.o ft2sec.o imaxoverflow.o intoverflow.o localtime.o \
             printelapse.o printisdst.o printreltm.o printtm.o printusage.o \
             sec2ft.o secoverflow.o yeardays.o

SETFT_OBJS=argempty.o argisdst.o argmatch.o argnumimax.o argnumint.o \
           argreltm.o argtmiso8601.o argweekday.o currentft.o errft.o \
           ft2sec.o ft2val.o getft.o localtime.o imaxoverflow.o intoverflow.o \
           mktime.o modifysec.o printelapse.o printusage.o sec2ft.o \
           secoverflow.o yeardays.o

# Rules compiling for Windows 64-bits to use self-implemented functions

%.o : %.c
	$(CC) $(CFLAGS) -DUSE_TM_SELFIMPL -o $@ -c $<

%_test.o : %.c
	$(CC) $(CFLAGS) -DUSE_TM_SELFIMPL -DTEST -o $@ -c $<

error_free.o : error.c
	$(CC) $(CFLAGS) -DUSE_TM_SELFIMPL -DFREE_WARGV -o $@ -c $<

libtouch.a: $(TOUCH_OBJS) adjustday.o adjusttm.o adjusttz.o encword.o error_free.o leapdays.o weekday.o yeardays.o
	$(AR) rcs $@ $^

libadjustday.a: $(ADJUSTDAY_OBJS)
	$(AR) rcs $@ $^

libcurrentft.a: $(CURRENTFT_OBJS)
	$(AR) rcs $@ $^

libgetft.a: $(GETFT_OBJS) error_free.o
	$(AR) rcs $@ $^

libleapdays.a: $(LEAPDAYS_OBJS)
	$(AR) rcs $@ $^

liblocaltime.a: $(LOCALTIME_OBJS) adjustday.o adjusttm.o adjusttz.o leapdays.o weekday.o
	$(AR) rcs $@ $^

libmktime.a: $(MKTIME_OBJS) adjustday.o adjusttm.o adjusttz.o leapdays.o secoverflow.o weekday.o
	$(AR) rcs $@ $^

libmodifysec.a: $(MODIFYSEC_OBJS)
	$(AR) rcs $@ $^

libparseft.a: $(PARSEFT_OBJS) adjustday.o adjusttm.o adjusttz.o leapdays.o encword.o weekday.o
	$(AR) rcs $@ $^

libsetft.a: $(SETFT_OBJS) adjustday.o adjusttm.o adjusttz.o error_free.o leapdays.o weekday.o
	$(AR) rcs $@ $^

# Rules compiling for Windows 32-bits to use self-implemented functions

%_win32.o : %.c
	$(CC_X86) $(CFLAGS) -DUSE_TM_SELFIMPL -o $@ -c $<

%_x86test.o : %.c
	$(CC_X86) $(CFLAGS) -DUSE_TM_SELFIMPL -DTEST -o $@ -c $<

error_free_win32.o : error.c
	$(CC_X86) $(CFLAGS) -DUSE_TM_SELFIMPL -DFREE_WARGV -o $@ -c $<

libx86touch.a: $(patsubst %.o,%_win32.o,$(TOUCH_OBJS)) adjustday_win32.o adjusttm_win32.o adjusttz_win32.o encword_win32.o error_free_win32.o leapdays_win32.o weekday_win32.o yeardays_win32.o
	$(AR) rcs $@ $^

libx86adjustday.a: $(patsubst %.o,%_win32.o,$(ADJUSTDAY_OBJS))
	$(AR) rcs $@ $^

libx86currentft.a: $(patsubst %.o,%_win32.o,$(CURRENTFT_OBJS))
	$(AR) rcs $@ $^

libx86getft.a: $(patsubst %.o,%_win32.o,$(GETFT_OBJS)) error_free_win32.o
	$(AR) rcs $@ $^

libx86leapdays.a: $(patsubst %.o,%_win32.o,$(LEAPDAYS_OBJS))
	$(AR) rcs $@ $^

libx86localtime.a: $(patsubst %.o,%_win32.o,$(LOCALTIME_OBJS)) adjustday_win32.o adjusttm_win32.o adjusttz_win32.o leapdays_win32.o weekday_win32.o
	$(AR) rcs $@ $^

libx86mktime.a: $(patsubst %.o,%_win32.o,$(MKTIME_OBJS)) adjustday_win32.o adjusttm_win32.o adjusttz_win32.o leapdays_win32.o secoverflow_win32.o weekday_win32.o
	$(AR) rcs $@ $^

libx86modifysec.a: $(patsubst %.o,%_win32.o,$(MODIFYSEC_OBJS))
	$(AR) rcs $@ $^

libx86parseft.a: $(patsubst %.o,%_win32.o,$(PARSEFT_OBJS)) adjustday_win32.o adjusttm_win32.o adjusttz_win32.o leapdays_win32.o encword_win32.o weekday_win32.o
	$(AR) rcs $@ $^

libx86setft.a: $(patsubst %.o,%_win32.o,$(SETFT_OBJS)) adjustday_win32.o adjusttm_win32.o adjusttz_win32.o error_free_win32.o leapdays_win32.o weekday_win32.o
	$(AR) rcs $@ $^

# Rules compiling for GNU/Linux to use POSIX functions in GNU C Library

%_glibc.o : %.c
	$(GCC) $(GFLAGS) -DUSE_TM_GLIBC -o $@ -c $<

%_gnutest.o : %.c
	$(GCC) $(GFLAGS) -DUSE_TM_GLIBC -DTEST -o $@ -c $<

libgnutouch.a: $(patsubst %.o,%_glibc.o,$(TOUCH_OBJS)) error_glibc.o fd-reopen_glibc.o fdutimensat_glibc.o
	$(AR) rcs $@ $^

libgnuadjustday.a: $(patsubst %.o,%_glibc.o,$(ADJUSTDAY_OBJS))
	$(AR) rcs $@ $^

libgnucurrentft.a: $(patsubst %.o,%_glibc.o,$(CURRENTFT_OBJS))
	$(AR) rcs $@ $^

libgnugetft.a: $(patsubst %.o,%_glibc.o,$(GETFT_OBJS)) error_glibc.o
	$(AR) rcs $@ $^

libgnuleapdays.a: $(patsubst %.o,%_glibc.o,$(LEAPDAYS_OBJS))
	$(AR) rcs $@ $^

libgnulocaltime.a: $(patsubst %.o,%_glibc.o,$(LOCALTIME_OBJS))
	$(AR) rcs $@ $^

libgnumktime.a: $(patsubst %.o,%_glibc.o,$(MKTIME_OBJS))
	$(AR) rcs $@ $^

libgnumodifysec.a: $(patsubst %.o,%_glibc.o,$(MODIFYSEC_OBJS))
	$(AR) rcs $@ $^

libgnuparseft.a: $(patsubst %.o,%_glibc.o,$(PARSEFT_OBJS))
	$(AR) rcs $@ $^

libgnusetft.a: $(patsubst %.o,%_glibc.o,$(SETFT_OBJS)) error_glibc.o fd-reopen_glibc.o fdutimensat_glibc.o
	$(AR) rcs $@ $^

# Rules compiling for Windows 64-bits to use POSIX functions in MS Visual
# C++ Runtime Library

%_msvcrt.o : %.c
	$(CC) $(CFLAGS) -DUSE_TM_MSVCRT -o $@ -c $<

%_mstest.o : %.c
	$(CC) $(CFLAGS) -DUSE_TM_MSVCRT -DTEST -o $@ -c $<

libmstouch.a: $(patsubst %.o,%_msvcrt.o,$(TOUCH_OBJS)) error_free.o encword.o tmdiff.o yeardays.o
	$(AR) rcs $@ $^

libmslocaltime.a: $(patsubst %.o,%_msvcrt.o,$(LOCALTIME_OBJS)) tmdiff.o
	$(AR) rcs $@ $^

libmsmktime.a: $(patsubst %.o,%_msvcrt.o,$(MKTIME_OBJS)) tmdiff.o
	$(AR) rcs $@ $^

libmsparseft.a: $(patsubst %.o,%_msvcrt.o,$(PARSEFT_OBJS)) encword.o tmdiff.o
	$(AR) rcs $@ $^

libmssetft.a: $(patsubst %.o,%_msvcrt.o,$(SETFT_OBJS)) error_free.o tmdiff.o
	$(AR) rcs $@ $^

clean:
	-rm -f *.o *.a
