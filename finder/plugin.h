/* plugin.h
 *
 * Implementation of a finder menu plugin for git-cheetah
 *
 * (c) by Heiko Voigt <hvoigt@hvoigt.net>
 *
 * inspired by an example from Brent Simmons
 * brent@ranchero.com, http://macte.ch/kmyXM
 *
 */

#ifndef FINDER_PLUGIN_H
#define FINDER_PLUGIN_H

/* on Mac OS 10.4 there is a struct strbuf in Carbon */
#define strbuf mac_strbuf
#include <Carbon/Carbon.h>
#undef strbuf
#include <CoreFoundation/CFPlugInCOM.h>
#include "../common/menuengine.h"

/* UUID of this plugin:
 * C46F90F0-A299-4AB4-857C-CC2B6D48EC25
 */
#define myUUID (CFUUIDGetConstantUUIDWithBytes (NULL, 0xC4, 0x6F, 0x90, \
			0xF0, 0xA2, 0x99, 0x4A, 0xB4, 0x85, 0x7C, 0xCC, \
			0x2B, 0x6D, 0x48, 0xEC, 0x25))

struct plugin_data {
	ContextualMenuInterfaceStruct *virtual_table;
	struct git_data git_data;
	CFUUIDRef id;
	UInt32 num_ref;
};

struct osx_menu_data {
	AEDescList *menu;
	char *menu_name;
	const AEDescList *selection;
	OSStatus status;
};

extern void *create_instance(CFAllocatorRef a, CFUUIDRef id);

#endif /* FINDER_PLUGIN_H */
