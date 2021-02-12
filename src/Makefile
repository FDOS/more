UPX=upx --8086 --best
# if you don't want to use UPX set
#     UPX=-rem (DOS)
#   or
#     UPX=true (Unix)
# if you use UPX: then options are
#     --8086 for 8086 compatibility
#   or
#     --best for smallest

all: more.exe

more.exe more.com: more.c ../kitten/kitten.c
	$(CC) $(CFLAGS)$@ $^ $(LDFLAGS)
	$(UPX) $@

clean:
	$(RM) more.exe
	$(RM) more.com
	$(RM) *.o
	$(RM) *.obj
