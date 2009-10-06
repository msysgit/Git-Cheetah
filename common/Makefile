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
