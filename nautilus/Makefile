TARGET = git_cheetah_ext
MODULES = menu.c util.c plugin.c ../compat/posix.c
OBJECTS = ${MODULES:%.c=%.o}
DESTDIR = /

LIBDIR = $(DESTDIR)/usr/lib
LDFLAGS = -shared \
	$(shell pkg-config --libs libnautilus-extension)

WFLAGS=-std=c99 -Wall -pedantic -Werror

# needed to get the proper declarations for glibc
COMPAT_CFLAGS = -D_POSIX_C_SOURCE=200112L -D_GNU_SOURCE
CFLAGS = $(WFLAGS) -fPIC -g $(COMPAT_CFLAGS) \
	$(shell pkg-config --cflags libnautilus-extension)

export CFLAGS

all: $(TARGET).so

COMMON_OBJ = ../common/cheetahmenu.o ../common/date.o ../common/debug.o \
	     ../common/exec.o ../common/menuengine.o \
	     ../common/sha1_file.o ../common/strbuf.o \
	     ../common/usage.o ../common/wrapper.o

common-obj:
	$(MAKE) -C ../common all

%.o : %.c
	$(CC) $(CFLAGS) $< -c -o $@

deps: $(MODULES)
	$(CC) $(CFLAGS) -MM $(MODULES) > deps

$(TARGET).so: $(OBJECTS) $(COMPAT_OBJ) common-obj
		gcc $(LDFLAGS) $(OBJECTS) $(COMPAT_OBJ) $(COMMON_OBJ) \
				-o $(TARGET).so

-include deps

install: all
	mkdir -p $(LIBDIR)/nautilus/extensions-2.0
	cp $(TARGET).so $(LIBDIR)/nautilus/extensions-2.0/

clean:
	rm -f $(OBJECTS) $(COMPAT_OBJ) $(TARGET).so deps
	$(MAKE) -C ../common clean
