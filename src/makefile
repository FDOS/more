# this is a Borland makefile


# set 
#   UPX=-rem 
# if you dont want to UPX choice

UPX=-rem
UPX=upx --8086 --best



LIBFILES=KITTEN.C TCDUMMY.C TALLOC.C PRF.C

############# WATCOM ########################
# -we treat all warnings as errors
# -wx set warning level to max
# -zq operate quietly
# -fm[=<map_file>]  generate map file
# -fe=<executable> name executable file
CC=wcl
CFLAGS=-oas -s -wx -we -zq -fm -fe=

############# TURBO_C ########################
# -w warn -M create map -f- no floating point -Z register optimize
# -O jump optimize -k- no standard stack frome -K unsigned char
# -exxx executable name (must be last) -mt tiny (default is small)
# -N stack checking -a- byte alignment  -ln no default libs
# -lt create .com file -lx no map file ...
CC=tcc
CFLAGS=-w -M -Z -O -k- -e

all: more.exe

more.exe: MORE.C $(LIBFILES)
 $(CC)  -DNOCATS  $(CFLAGS)_more MORE.C $(LIBFILES)
 $(UPX) _more.exe
 $(CC)            $(CFLAGS)more  MORE.C $(LIBFILES)
 $(UPX) more.exe

realclean clean:
	-del *.obj

distclean: clean
	-del *.exe
