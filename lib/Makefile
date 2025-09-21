GCC=gcc
CC=i686-w64-mingw32-gcc
CFLAGS=-std=c99 -O2 -Wformat -Werror

ADJUSTDAY_OBJS=adjusttm.o error.o intoverflow.o leapdays.o printtm.o \
               sscantmint.o weekday.o yeardays.o

GETTIME_OBJS=ft2secns.o printtm.o timeoverflow.o yeardays.o

LOCALTIME_OBJS=error.o imaxoverflow.o intoverflow.o printtm.o sscantmimax.o \
               timeoverflow.o yeardays.o

MKTIME_OBJS=error.o intoverflow.o printtm.o sscantmint.o yeardays.o

# Rules compiling for Mingw to use wrapper localtime or mkfile function
%.o : %.c
	$(CC) $(CFLAGS) -DUSE_TM_WRAPPER -o $@ -c $<

%_test.o : %.c
	$(CC) $(CFLAGS) -DUSE_TM_WRAPPER -DTEST -o $@ -c $<

libadjustday.a: $(ADJUSTDAY_OBJS)
	$(AR) rcs $@ $^

libgettime.a: $(GETTIME_OBJS)
	$(AR) rcs $@ $^

liblocaltime.a: $(LOCALTIME_OBJS) adjustday.o adjusttm.o adjusttz.o leapdays.o weekday.o
	$(AR) rcs $@ $^

libmktime.a: $(MKTIME_OBJS) adjustday.o adjusttm.o adjusttz.o imaxoverflow.o leapdays.o timeoverflow.o weekday.o
	$(AR) rcs $@ $^

# Rules compiling for Cygwin to use localtime_r or mkfile function
%_cygwin.o : %.c
	$(GCC) $(CFLAGS) -DUSE_TM_CYGWIN -o $@ -c $<

%_cygtest.o : %.c
	$(GCC) $(CFLAGS) -DUSE_TM_CYGWIN -DTEST -o $@ -c $<

libcyggettime.a: $(patsubst %.o,%_cygwin.o,$(GETTIME_OBJS)) secns2ft_cygwin.o
	$(AR) rcs $@ $^

libcyglocaltime.a: $(patsubst %.o,%_cygwin.o,$(LOCALTIME_OBJS))
	$(AR) rcs $@ $^

libcygmktime.a: $(patsubst %.o,%_cygwin.o,$(MKTIME_OBJS))
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
