GCC=gcc
LD=x86_64-w64-mingw32-gcc
LD_X86=i686-w64-mingw32-gcc
LDFLAGS=-mconsole

all: adjustday currentft getft leapdays localtime mktime modifysec parseft setft

x86: x86adjustday x86currentft x86getft x86leapdays x86localtime x86mktime x86modifysec x86parseft x86setft

glibc: gnuadjustday gnucurrentft gnugetft gnuleapdays gnulocaltime gnumktime gnumodifysec gnuparseft gnusetft

msvcrt: mslocaltime msmktime msparseft mssetft

.PHONY: adjustday currentft getft leapdays localtime mktime modifysec parseft setft

adjustday currentft getft leapdays localtime mktime modifysec parseft setft:
	(cd lib && $(MAKE) $@_test.o lib$@.a)
	mkdir -p bin
	$(LD) $(LDFLAGS) -o bin/$@ lib/$@_test.o lib/lib$@.a

.PHONY: x86adjustday x86currentft x86getft x86leapdays x86localtime x86mktime x86modifysec x86parseft x86setft

x86adjustday x86currentft x86getft x86leapdays x86localtime x86mktime x86modifysec x86parseft x86setft:
	(cd lib && $(MAKE) $(subst x86,,$@)_x86test.o lib$@.a)
	mkdir -p win32
	$(LD_X86) -o win32/$(subst x86,,$@) lib/$(subst x86,,$@)_x86test.o lib/lib$@.a

.PHONY: gnuadjustday gnucurrentft gnugetft gnuleapdays gnulocaltime gnumktime gnumodifysec gnuparseft gnusetft

gnuadjustday gnucurrentft gnugetft gnuleapdays gnulocaltime gnumktime gnumodifysec gnuparseft gnusetft:
	(cd lib && $(MAKE) $(subst gnu,,$@)_gnutest.o lib$@.a)
	mkdir -p glibc
	$(GCC) -o glibc/$(subst gnu,,$@) lib/$(subst gnu,,$@)_gnutest.o lib/lib$@.a

.PHONY: mslocaltime msmktime msparseft mssetft

mslocaltime msmktime msparseft mssetft:
	(cd lib && $(MAKE) $(subst ms,,$@)_mstest.o lib$@.a)
	mkdir -p msvcrt
	$(LD) $(LDFLAGS) -o msvcrt/$(subst ms,,$@) lib/$(subst ms,,$@)_mstest.o lib/lib$@.a

clean:
#	(cd src && $(MAKE) $@)
	(cd lib && $(MAKE) $@)
