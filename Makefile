OBJECTS=ext.o debug.o dll.o factory.o menu.o systeminfo.o registry.o exec.o
CFLAGS=-O -g

TARGET=git_shell_ext.dll
MSYSGIT_PATH=$(shell cd /; pwd -W | sed -e 's|/|\\\\\\\\|g')
DLL_PATH=$(shell pwd -W | sed -e 's|/|\\\\\\\\|g')\\\\$(TARGET)

all: $(TARGET)

.o:.c
	$(CC) $(CFLAGS) $< -o $@

$(TARGET): $(OBJECTS) git_shell_ext.def
	dllwrap.exe --enable-stdcall-fixup --def git_shell_ext.def \
		$(OBJECTS) -o $@ -luuid -loleaut32 -lole32

#	gcc $(LDFLAGS) -o $@ $(OBJECTS)  -lole32 -luuid -loleaut32
#	dlltool -d git_shell_ext.def -l $@ $(OBJECTS)

dll.o: dll.h ext.h factory.h systeminfo.h registry.h
ext.o: ext.h debug.h
factory.o: factory.h ext.h menu.h
menu.o: menu.h ext.h debug.h systeminfo.h exec.h
systeminfo.o: systeminfo.h
registry.o: registry.h
exec.o: debug.h systeminfo.h exec.h

install: all
	regsvr32 -s -n -i:machine $(DLL_PATH)

uninstall: all
	regsvr32 -u -s -n -i:machine $(DLL_PATH)

install-user: all
	regsvr32 -s $(DLL_PATH)

uninstall-user: all
	regsvr32 -u -s $(DLL_PATH)

clean:
	-rm -f $(OBJECTS) $(TARGET)
