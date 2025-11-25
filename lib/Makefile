CC=x86_64-w64-mingw32-gcc
CC_X86=i686-w64-mingw32-gcc
CFLAGS=-std=c11 -O2 -Wformat -Werror
GCC=gcc
GFLAGS=-std=gnu11 -O2 -Wformat -Werror

ADJUSTDAY_OBJS=adjusttm.o error.o imaxoverflow.o intoverflow.o leapdays.o \
               printtm.o printusage.o sscannumimax.o sscannumint.o \
               sscanreltm.o weekday.o yeardays.o

GETFT_OBJS=currentft.o errft.o ft2sec.o ft2val.o imaxoverflow.o intoverflow.o \
           modifysec.o printelapse.o printusage.o secoverflow.o sscannumint.o

CURRENTFT_OBJS=error.o ft2sec.o ft2val.o imaxoverflow.o modifysec.o \
               printelapse.o printusage.o secoverflow.o yeardays.o

LEAPDAYS_OBJS=error.o intoverflow.o printusage.o sscannumint.o

LOCALTIME_OBJS=error.o imaxoverflow.o intoverflow.o printisdst.o printtm.o \
               printusage.o secoverflow.o sscannumimax.o sscannumint.o \
               sscanseconds.o yeardays.o

MKTIME_OBJS=error.o imaxoverflow.o intoverflow.o printelapse.o printisdst.o \
            printtm.o printusage.o sscanisdst.o sscannumimax.o sscannumint.o \
            sscanreltm.o sscanword.o yeardays.o

MODIFYSEC_OBJS=currentft.o error.o imaxoverflow.o intoverflow.o printelapse.o \
               printusage.o secoverflow.o sscannumimax.o sscannumint.o \
               sscanseconds.o

# Rules compiling for Windows 64-bits to use self-implemented functions
%.o : %.c
	$(CC) $(CFLAGS) -DUSE_TM_SELFIMPL -o $@ -c $<

%_test.o : %.c
	$(CC) $(CFLAGS) -DUSE_TM_SELFIMPL -DTEST -o $@ -c $<

error_free.o : error.c
	$(CC) $(CFLAGS) -DUSE_TM_SELFIMPL -DFREE_WARGV -o $@ -c $<

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

# Rules compiling for Windows 32-bits to use self-implemented functions
%_win32.o : %.c
	$(CC_X86) $(CFLAGS) -DUSE_TM_SELFIMPL -o $@ -c $<

%_x86test.o : %.c
	$(CC_X86) $(CFLAGS) -DUSE_TM_SELFIMPL -DTEST -o $@ -c $<

error_free_win32.o : error.c
	$(CC_X86) $(CFLAGS) -DUSE_TM_SELFIMPL -DFREE_WARGV -o $@ -c $<

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

# Rules compiling for GNU/Linux to use POSIX functions in GNU C Library
%_glibc.o : %.c
	$(GCC) $(GFLAGS) -DUSE_TM_GLIBC -o $@ -c $<

%_gnutest.o : %.c
	$(GCC) $(GFLAGS) -DUSE_TM_GLIBC -DTEST -o $@ -c $<

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

# Rules compiling for Windows 64-bits to use POSIX functions in MS Visual
# C++ Runtime Library
%_msvcrt.o : %.c
	$(CC) $(CFLAGS) -DUSE_TM_MSVCRT -o $@ -c $<

%_mstest.o : %.c
	$(CC) $(CFLAGS) -DUSE_TM_MSVCRT -DTEST -o $@ -c $<

libmslocaltime.a: $(patsubst %.o,%_msvcrt.o,$(LOCALTIME_OBJS)) tmdiff_msvcrt.o
	$(AR) rcs $@ $^

libmsmktime.a: $(patsubst %.o,%_msvcrt.o,$(MKTIME_OBJS)) tmdiff_msvcrt.o
	$(AR) rcs $@ $^

clean:
	-rm -f *.o *.a
