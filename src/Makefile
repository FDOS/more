
all: more.exe

# Watcom has different flag for output file name at compile vs link
_CFLAGS = $(subst -fe=,-fo=,$(CFLAGS))

kitten.obj: ../kitten/kitten.c
	$(CC) $(_CFLAGS)$@ -c $^

tnyprntf.obj: ../tnyprntf/tnyprntf.c
	$(CC) $(_CFLAGS)$@ -c $^

more.exe more.com: more.c $(EXTRA_OBJS)
	$(CC) $(CFLAGS)$@ $^ $(LDFLAGS)
	$(UPXARGS) $@

clean:
	$(RM) more.exe
	$(RM) more.com
	$(RM) *.o
	$(RM) *.obj
	$(RM) *.map
