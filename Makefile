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

dll.o: dll.h ext.h factory.h
ext.o: ext.h debug.h
factory.o: factory.h ext.h menu.h
menu.o: menu.h ext.h debug.h systeminfo.h
systeminfo.o: systeminfo.h

install%: install%.reg all
	regsvr32 -s $(DLL_PATH)
	regedit -s $<

uninstall%: uninstall%.reg
	regsvr32 -u -s $(DLL_PATH)
	regedit -s $<

install.reg: install.reg.in Makefile
	sed < $< > $@ \
		-e 's|@@MSYSGIT_PATH@@|$(MSYSGIT_PATH)|' \
		-e 's|@@DLL_PATH@@|$(DLL_PATH)|'

%-user.reg: %.reg
	sed -e 's|HKEY_CLASSES_ROOT\\|HKEY_CURRENT_USER\\Software\\Classes\\|' \
		-e 's|HKEY_LOCAL_MACHINE\\|HKEY_CURRENT_USER\\|' \
		< $< > $@

clean:
	-rm -f $(OBJECTS) $(TARGET)
