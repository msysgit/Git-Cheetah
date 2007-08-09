OBJECTS=ext.o debug.o dll.o factory.o menu.o systeminfo.o
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

install: all install.reg
	regsvr32 -s git_shell_ext.dll
	regedit -s install.reg

uninstall:
	regsvr32 -u -s git_shell_ext.dll
	regedit -s uninstall.reg

install.reg: install.reg.in Makefile
	sed < $< > $@ \
		-e 's|@@MSYSGIT_PATH@@|$(MSYSGIT_PATH)|' \
		-e 's|@@DLL_PATH@@|$(DLL_PATH)|'

clean:
	-rm -f $(OBJECTS) $(TARGET)
