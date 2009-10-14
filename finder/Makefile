CFLAGS = -g -pedantic -Wall -std=c99
TARGET = git-cheetah
MODULES = menu.c util.c plugin.c ../compat/posix.c

OBJECTS=${MODULES:%.c=%.o}

COMMON_OBJ = ../common/cheetahmenu.o ../common/date.o ../common/debug.o \
	     ../common/exec.o ../common/menuengine.o \
	     ../common/sha1_file.o ../common/strbuf.o \
	     ../common/usage.o ../common/wrapper.o

export CFLAGS

all: $(TARGET).plugin

common-obj:
	$(MAKE) -C ../common all

%.o : %.c
	$(CC) $(CFLAGS) $< -c -o $@

deps: $(MODULES)
	$(CC) $(CFLAGS) -MM $(MODULES) > deps

$(TARGET): $(OBJECTS) common-obj
	gcc $(OBJECTS) $(COMMON_OBJ) \
		-framework CoreFoundation \
		-framework ApplicationServices \
		-framework Carbon \
		-bundle -o git-cheetah

$(TARGET).plugin: $(TARGET).plugin/Contents/MacOS/$(TARGET)

$(TARGET).plugin/Contents/MacOS/$(TARGET): $(TARGET) \
					   $(TARGET).plugin/Contents
	mkdir -p $(TARGET).plugin/Contents/MacOS
	cp $(TARGET) $(TARGET).plugin/Contents/MacOS/

$(TARGET).plugin/Contents: PkgInfo Info.plist
	mkdir -p $(TARGET).plugin/Contents
	cp PkgInfo $(TARGET).plugin/Contents/
	cp Info.plist $(TARGET).plugin/Contents/

-include deps

install: all
	rm -rf ~/Library/Contextual\ Menu\ Items/$(TARGET).plugin
	cp -r $(TARGET).plugin ~/Library/Contextual\ Menu\ Items/$(TARGET).plugin

clean:
	-rm -f $(OBJECTS) $(TARGET) deps
	-rm -rf $(TARGET).plugin
	$(MAKE) -C ../common clean
