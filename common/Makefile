ifneq (,$(W64))
cross = /src/mingw-w64/sysroot/bin/x86_64-w64-mingw32-
CC = $(cross)gcc.exe
endif

MODULES=debug.c exec.c menuengine.c cheetahmenu.c date.c \
	sha1_file.c strbuf.c usage.c wrapper.c
OBJECTS=${MODULES:%.c=%.o}

all: $(OBJECTS)

%.o : %.c
	$(CC) $(CFLAGS) $< -c -o $@

deps: $(MODULES)
	$(CC) $(CFLAGS) -MM $(MODULES) > deps

-include deps

clean:
	-rm -f $(OBJECTS) deps
