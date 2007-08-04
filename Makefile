OBJECTS=git_shell_ext.o

all: git_shell_ext.dll

git_shell_ext.dll: $(OBJECTS) git_shell_ext.def
	dllwrap.exe --enable-stdcall-fixup --def git_shell_ext.def \
		$(OBJECTS) -o $@ -luuid -loleaut32 -lole32

#	gcc $(LDFLAGS) -o $@ $(OBJECTS)  -lole32 -luuid -loleaut32
#	dlltool -d git_shell_ext.def -l $@ $(OBJECTS)

install:
	regsvr32 git_shell_ext.dll
	regedit install.reg

uninstall:
	regsvr32 -u git_shell_ext.dll
	regedit uninstall.reg
clean:
	rm $(OBJECTS) git_shell_ext.dll