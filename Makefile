GCC=gcc
LD=i686-w64-mingw32-gcc
LDFLAGS=-mconsole

all: adjustday gettime localtime mktime

cygwin: cyggettime cyglocaltime cygmktime

msvcrt: mslocaltime msmktime

.PHONY: adjustday gettime localtime mktime

adjustday gettime localtime mktime:
	(cd lib && $(MAKE) $@_test.o lib$@.a)
	$(LD) $(LDFLAGS) -o $@ lib/$@_test.o lib/lib$@.a

.PHONY: cyggettime cyglocaltime cygmktime

cyggettime cyglocaltime cygmktime:
	(cd lib && $(MAKE) $(subst cyg,,$@)_cygtest.o lib$@.a)
	mkdir -p cygwin
	$(GCC) -o cygwin/$(subst cyg,,$@) lib/$(subst cyg,,$@)_cygtest.o lib/lib$@.a

.PHONY: mslocaltime msmktime

mslocaltime msmktime:
	(cd lib && $(MAKE) $(subst ms,,$@)_mstest.o lib$@.a)
	mkdir -p msvcrt
	$(LD) $(LDFLAGS) -o msvcrt/$(subst ms,,$@) lib/$(subst ms,,$@)_mstest.o lib/lib$@.a

clean:
	(cd lib && $(MAKE) $@)
