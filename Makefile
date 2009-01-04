OBJECTS=ext.o debug.o dll.o factory.o menu.o systeminfo.o registry.o exec.o
CFLAGS=-O -g -DNO_MMAP -DNO_PREAD -DNO_STRLCPY
COMPAT_H = cache.h git-compat-util.h hash.h strbuf.h compat/mingw.h
COMPAT_OBJ = date.o sha1_file.o strbuf.o usage.o wrapper.o \
	compat/mingw.o compat/mmap.o compat/pread.o compat/strlcpy.o \
	compat/winansi.o

TARGET=git_shell_ext.dll
MSYSGIT_PATH=$(shell cd /; pwd -W | sed -e 's|/|\\\\\\\\|g')
DLL_PATH=$(shell pwd -W | sed -e 's|/|\\\\\\\\|g')\\\\$(TARGET)

all: $(TARGET)

.o:.c
	$(CC) $(CFLAGS) $< -o $@

$(TARGET): $(OBJECTS) $(COMPAT_OBJ) git_shell_ext.def
	dllwrap.exe --enable-stdcall-fixup --def git_shell_ext.def \
		$(OBJECTS) $(COMPAT_OBJ) -o $@ -luuid -loleaut32 -lole32 -lws2_32

#	gcc $(LDFLAGS) -o $@ $(OBJECTS)  -lole32 -luuid -loleaut32
#	dlltool -d git_shell_ext.def -l $@ $(OBJECTS)

dll.o: dll.h ext.h factory.h systeminfo.h registry.h menuengine.h
ext.o: ext.h debug.h systeminfo.h menuengine.h
factory.o: factory.h ext.h menu.h menuengine.h
menu.o: menu.h ext.h debug.h systeminfo.h exec.h menuengine.h cheetahmenu.h
systeminfo.o: systeminfo.h
registry.o: registry.h
exec.o: debug.h systeminfo.h exec.h
$(COMPAT_OBJ) : $(COMPAT_H)

install: all
	regsvr32 -s -n -i:machine $(DLL_PATH)

uninstall: all
	regsvr32 -u -s -n -i:machine $(DLL_PATH)

install-user: all
	regsvr32 -s $(DLL_PATH)

uninstall-user: all
	regsvr32 -u -s $(DLL_PATH)

clean:
	-rm -f $(OBJECTS) $(COMPAT_OBJ) $(TARGET)
