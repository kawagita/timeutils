CC=i686-w64-mingw32-gcc
CFLAGS=-std=c11 -O2 -Wformat -Werror
GCC=gcc
GFLAGS=-std=gnu11 -O2 -Wformat -Werror

ADJUSTDAY_OBJS=adjusttm.o error.o imaxoverflow.o intoverflow.o leapdays.o \
               printtm.o printusage.o sscannumimax.o sscannumint.o sscantm.o \
               sscanword.o weekday.o yeardays.o

GETFT_OBJS=currentft.o errft.o ft2sec.o imaxoverflow.o intoverflow.o \
           modifysec.o printtm.o printusage.o sec2ft.o sscannumint.o \
           timeoverflow.o toftval.o yeardays.o

CURRENTFT_OBJS=error.o ft2sec.o imaxoverflow.o modifysec.o printtm.o \
               printusage.o sec2ft.o timeoverflow.o toftval.o yeardays.o

LEAPDAYS_OBJS=error.o intoverflow.o printusage.o sscannumint.o

LOCALTIME_OBJS=error.o imaxoverflow.o intoverflow.o printtm.o printusage.o \
               sscannumimax.o sscannumint.o sscantm.o sscanword.o \
               timeoverflow.o yeardays.o

MKTIME_OBJS=error.o imaxoverflow.o intoverflow.o printtm.o printusage.o \
            sscannumimax.o sscannumint.o sscantm.o sscanword.o yeardays.o

MODIFYSEC_OBJS=error.o ft2sec.o currentft.o imaxoverflow.o intoverflow.o \
               modifysec.o printtm.o printusage.o sec2ft.o sscannumimax.o \
               sscannumint.o sscantm.o sscanword.o timeoverflow.o toftval.o \
               yeardays.o

# Rules compiling for Mingw to use wrapper localtime or mkfile function
%.o : %.c
	$(CC) $(CFLAGS) -DUSE_TM_WRAPPER -o $@ -c $<

%_test.o : %.c
	$(CC) $(CFLAGS) -DUSE_TM_WRAPPER -DTEST -o $@ -c $<

error_free.o : error.c
	$(CC) $(CFLAGS) -DUSE_TM_WRAPPER -DFREE_WARGV -o $@ -c $<

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

libmktime.a: $(MKTIME_OBJS) adjustday.o adjusttm.o adjusttz.o leapdays.o timeoverflow.o weekday.o
	$(AR) rcs $@ $^

libmodifysec.a: $(MODIFYSEC_OBJS)
	$(AR) rcs $@ $^

# Rules compiling for Cygwin to use localtime_r or mkfile function
%_cygwin.o : %.c
	$(GCC) $(GFLAGS) -DUSE_TM_CYGWIN -o $@ -c $<

%_cygtest.o : %.c
	$(GCC) $(GFLAGS) -DUSE_TM_CYGWIN -DTEST -o $@ -c $<

libcygcurrentft.a: $(patsubst %.o,%_cygwin.o,$(CURRENTFT_OBJS))
	$(AR) rcs $@ $^

libcyggetft.a: $(patsubst %.o,%_cygwin.o,$(GETFT_OBJS)) error_cygwin.o
	$(AR) rcs $@ $^

libcyglocaltime.a: $(patsubst %.o,%_cygwin.o,$(LOCALTIME_OBJS))
	$(AR) rcs $@ $^

libcygmktime.a: $(patsubst %.o,%_cygwin.o,$(MKTIME_OBJS))
	$(AR) rcs $@ $^

libcygmodifysec.a: $(patsubst %.o,%_cygwin.o,$(MODIFYSEC_OBJS))
	$(AR) rcs $@ $^

# Rules compiling for Mingw to use localtime_s or mkfile function in MSVCRT
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
