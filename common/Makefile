ifneq (,$(W64))
cross = /src/mingw-w64/sysroot/bin/x86_64-w64-mingw32-
CC = $(cross)gcc.exe
endif

MODULES=debug.c exec.c menuengine.c cheetahmenu.c date.c \
	sha1_file.c strbuf.c usage.c wrapper.c
OBJECTS=${MODULES:%.c=%.o} version.h

# transform descriptions of the form v1.0-124-g12345 into 1,0
# regex to limit the number of groups to four
UPTO_FOUR_WORDS=\([0-9]*\),\([0-9]*\),\([0-9]*\),\([0-9]*\).*

DESCRIBE=$(shell git describe)
ifeq ($(DESCRIBE),)
	VERSION="0,0,0,0"
else
	VERSION=$(shell echo $(DESCRIBE) | \
		sed -e 's/v\([0-9.]*\).*/\1/' -e 's/\./,/g' \
		-e 's/$(UPTO_FOUR_WORDS)/\1,\2,\3,\4/' \
		-e 's/\(.*\),$$/\1/')
endif

all: $(OBJECTS)

%.o : %.c
	$(CC) $(CFLAGS) $< -c -o $@

deps: $(MODULES)
	$(CC) $(CFLAGS) -MM $(MODULES) > deps

-include deps

clean:
	-rm -f $(OBJECTS) deps

.PHONY: version.h
version.h:
#	make sure that there are exactly 4 numbers, separated by commas
#	otherwise you'll receive "windres.exe: cheetah.rc:68: syntax error"
	@case "$(VERSION)" in \
		*,*,*,*) VERSION=$(VERSION) ;; \
		*,*,*) VERSION=$(VERSION),0 ;; \
		*,*) VERSION=$(VERSION),0,0 ;; \
		*) VERSION=$(VERSION),0,0,0 ;; \
	esac ; \
	sed -e "s/@@GIT_CHEETAH_VERSION@@/$$VERSION/" \
		-e "s/@@GIT_CHEETAH_COMMENTS@@/$(DESCRIBE)/" \
		version.in > version.h
