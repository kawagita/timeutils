CC=x86_64-w64-mingw32-gcc
CC_X86=i686-w64-mingw32-gcc
CFLAGS=-std=c11 -O2 -Wformat -Werror
GCC=gcc
GFLAGS=-std=gnu11 -O2 -Wformat -Werror

INCLUDE=-I../lib

%.o : %.c
	$(CC) $(INCLUDE) $(CFLAGS) -DUSE_TM_SELFIMPL -DUNICODE -o $@ -c $<

%_win32.o : %.c
	$(CC_X86) $(INCLUDE) $(CFLAGS) -DUSE_TM_SELFIMPL -DUNICODE -o $@ -c $<

%_glibc.o : %.c
	$(GCC) $(INCLUDE) $(GFLAGS) -DUSE_TM_GLIBC -o $@ -c $<

%_msvcrt.o : %.c
	$(CC) $(INCLUDE) $(CFLAGS) -DUSE_TM_MSVCRT -DUNICODE -o $@ -c $<

clean:
	-rm -f *.o
