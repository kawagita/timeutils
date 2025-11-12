GCC=gcc
LD=i686-w64-mingw32-gcc
LDFLAGS=-mconsole

all: adjustday currentft getft leapdays localtime mktime modifysec

cygwin: cygcurrentft cyggetft cyglocaltime cygmktime cygmodifysec

msvcrt: mslocaltime msmktime

.PHONY: adjustday currentft getft leapdays localtime mktime modifysec

adjustday currentft getft leapdays localtime mktime modifysec:
	(cd lib && $(MAKE) $@_test.o lib$@.a)
	$(LD) $(LDFLAGS) -o $@ lib/$@_test.o lib/lib$@.a

.PHONY: cygcurrentft cyggetft cyglocaltime cygmktime cygmodifysec

cygcurrentft cyggetft cyglocaltime cygmktime cygmodifysec:
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
