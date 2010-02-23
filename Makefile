# The default target of this Makefile is...
all::

uname_S := $(shell sh -c 'uname -s 2>/dev/null || echo not')

ifeq ($(uname_S),Linux)
	SUBDIR=nautilus
endif
ifeq ($(uname_S),Darwin)
	SUBDIR=finder
endif
ifneq (,$(findstring MINGW,$(uname_S)))
	SUBDIR=explorer
endif

TARGETS=all install uninstall install-user uninstall-user clean

$(TARGETS)::
	$(MAKE) -C $(SUBDIR) $@
