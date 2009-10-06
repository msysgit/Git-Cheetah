MODULES=ext.c dll.c factory.c menu.c systeminfo.c registry.c \
	columns.c
OBJECTS=${MODULES:%.c=%.o}
COMMON_OBJ = common/cheetahmenu.o common/date.o common/debug.o \
	     common/exec.o common/menuengine.o \
	     common/sha1_file.o common/strbuf.o \
	     common/usage.o common/wrapper.o
COMPAT_OBJ = compat/mingw.o compat/mmap.o compat/pread.o \
	     compat/strlcpy.o compat/winansi.o

ifeq ($(shell uname -o 2>/dev/null), Cygwin)
	OSCFLAGS =-mno-cygwin  -mwin32 -mdll
	OSDLLWRAPFLAG =-mno-cygwin  --target=i386-mingw32
endif

# define _WIN32_IE, so IColumnProvider's structures are available
CFLAGS=-O -g -DNO_MMAP -DNO_PREAD -DNO_STRLCPY -D_WIN32_IE=0x0500 $(OSCFLAGS)
DLLWRAPFLAGS = -Wl,--enable-stdcall-fixup $(OSDLLWRAPFLAG)

TARGET=git_shell_ext.dll
MSYSGIT_PATH=$(shell cd /; pwd -W | sed -e 's|/|\\\\\\\\|g')
DLL_PATH=$(shell pwd -W | sed -e 's|/|\\\\\\\\|g')\\\\$(TARGET)

# export compile flags to sub-make of the common folder
export CFLAGS

all: $(TARGET)

common-obj:
	$(MAKE) -C common all

%.o : %.c
	$(CC) $(CFLAGS) $< -c -o $@

deps: $(MODULES)
	$(CC) $(CFLAGS) -MM $(MODULES) > deps

$(TARGET): common-obj $(OBJECTS) $(COMPAT_OBJ) deps git_shell_ext.def
	dllwrap.exe $(DLLWRAPFLAGS) --def git_shell_ext.def \
		$(COMMON_OBJ) $(OBJECTS) $(COMPAT_OBJ) -o $@ \
		-luuid -loleaut32 -lole32 -lws2_32

#	gcc $(LDFLAGS) -o $@ $(OBJECTS)  -lole32 -luuid -loleaut32
#	dlltool -d git_shell_ext.def -l $@ $(OBJECTS)

-include deps

install: all
	regsvr32 -s -n -i:machine $(DLL_PATH)

uninstall: all
	regsvr32 -u -s -n -i:machine $(DLL_PATH)

install-user: all
	regsvr32 -s $(DLL_PATH)

uninstall-user: all
	regsvr32 -u -s $(DLL_PATH)

clean:
	-rm -f $(OBJECTS) $(COMPAT_OBJ) $(TARGET) deps
	$(MAKE)  -C common clean
