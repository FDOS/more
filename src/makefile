# this is a Borland makefile


# set 
#   UPX=-rem 
# if you dont want to UPX choice

UPX=-rem
UPX=upx --8086 --best


LIBFILES=../kitten/kitten.c tcdummy.c talloc.c prf.c


############# WATCOM ########################
# -we treat all warnings as errors
# -wx set warning level to max
# -zq operate quietly
# -fm[=<map_file>]  generate map file
# -fe=<executable> name executable file
CC=wcl
CFLAGS=-oas -s -wx -we -zq -fm -fe=

############# TURBO_C ########################

# CC=tcc
# CFLAGS=-w -M -Z -O -k- -e

all: more.exe

more.exe: more.c $(LIBFILES)
 $(CC)  -DNOCATS  $(CFLAGS)_more more.c $(LIBFILES)
# $(UPX) _more.exe
 $(CC)            $(CFLAGS)more  more.c $(LIBFILES)
# $(UPX) more.exe

realclean clean:
	-del *.obj

distclean: clean
	-del *.exe
