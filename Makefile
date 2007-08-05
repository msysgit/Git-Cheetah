OBJECTS=git_shell_ext.o git_shell_ext_debug.o
CFLAGS=-O -g

all: git_shell_ext.dll

.o:.c
	$(CC) $(CFLAGS) $< -o $@

git_shell_ext.dll: $(OBJECTS) git_shell_ext.def
	dllwrap.exe --enable-stdcall-fixup --def git_shell_ext.def \
		$(OBJECTS) -o $@ -luuid -loleaut32 -lole32

#	gcc $(LDFLAGS) -o $@ $(OBJECTS)  -lole32 -luuid -loleaut32
#	dlltool -d git_shell_ext.def -l $@ $(OBJECTS)

install: all
	regsvr32 -s git_shell_ext.dll
	regedit -s install.reg

uninstall:
	regsvr32 -u -s git_shell_ext.dll
	regedit -s uninstall.reg
clean:
	-rm -f $(OBJECTS) git_shell_ext.dll