TARGET64=git_shell_ext64.dll
TARGET32=git_shell_ext.dll
ifneq (,$(W64))
cross = /src/mingw-w64/sysroot/bin/x86_64-w64-mingw32-
WIN32_IE=0x0501
TARGET=$(TARGET64)
else
cross = ""
WIN32_IE=0x0500
TARGET=$(TARGET32)
endif

CC = $(cross)gcc.exe
DLLWRAP = $(cross)dllwrap.exe
WINDRES = $(cross)windres

MODULES=ext.c dll.c factory.c menu.c systeminfo.c registry.c \
	columns.c
OBJECTS=${MODULES:%.c=%.o} cheetah.o
COMMON_OBJ = ../common/cheetahmenu.o ../common/date.o ../common/debug.o \
	     ../common/exec.o ../common/menuengine.o \
	     ../common/sha1_file.o ../common/strbuf.o \
	     ../common/usage.o ../common/wrapper.o
COMPAT_OBJ = ../compat/mingw.o ../compat/mmap.o ../compat/pread.o \
	     ../compat/strlcpy.o ../compat/winansi.o

ifeq ($(shell uname -o 2>/dev/null), Cygwin)
	OSCFLAGS =-mno-cygwin  -mwin32 -mdll
	OSDLLWRAPFLAG =-mno-cygwin  --target=i386-mingw32
        DLL_PATH=$(shell cygpath -w "`pwd`" | sed -e 's|\\|\\\\\\\\|g')\\\\$(TARGET)
else
        DLL_PATH=$(shell pwd -W | sed -e 's|/|\\\\\\\\|g')\\\\$(TARGET)
endif

# define _WIN32_IE, so IColumnProvider's structures are available
DEFS=-DNO_MMAP -DNO_PREAD -DNO_STRLCPY -DSTRICT \
	-DWINVER=0x0500 -D_WIN32_WINNT=0x0500 -D_WIN32_IE=$(WIN32_IE)
CFLAGS=-O -g $(DEFS) $(OSCFLAGS)
DLLWRAPFLAGS = -Wl,--enable-stdcall-fixup $(OSDLLWRAPFLAG)

MSYSGIT_PATH=$(shell cd /; pwd -W | sed -e 's|/|\\\\\\\\|g')

# export compile flags to sub-make of the common folder
export CFLAGS

all: $(TARGET)

common-obj:
	$(MAKE) -C ../common all

%.o : %.c
	$(CC) $(CFLAGS) $< -c -o $@

deps: $(MODULES)
	$(CC) $(CFLAGS) -MM $(MODULES) > deps

$(TARGET): common-obj $(OBJECTS) $(COMPAT_OBJ) deps git_shell_ext.def
	$(DLLWRAP) $(DLLWRAPFLAGS) --def git_shell_ext.def \
		$(COMMON_OBJ) $(OBJECTS) $(COMPAT_OBJ) -o $@ \
		-luuid -loleaut32 -lole32 -lws2_32

#	gcc $(LDFLAGS) -o $@ $(OBJECTS)  -lole32 -luuid -loleaut32
#	dlltool -d git_shell_ext.def -l $@ $(OBJECTS)

-include deps

install: all
	regsvr32 -s -n -i:machine $(DLL_PATH)

uninstall:
	regsvr32 -u -s -n -i:machine $(DLL_PATH)

install-user: all
	regsvr32 -s $(DLL_PATH)

uninstall-user:
	regsvr32 -u -s $(DLL_PATH)

objects-clean:
	-rm -f $(OBJECTS) $(COMPAT_OBJ) deps
	$(MAKE) -C ../common clean

platform-clean: objects-clean
	-rm -f $(TARGET)

clean: platform-clean
	-rm -f  $(TARGET32) $(TARGET64)

cheetah.o: cheetah.rc ../common/version.h
	$(WINDRES) -o $@ -i $<
