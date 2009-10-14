/* plugin.c
 *
 * Implementation of a finder menu plugin for git-cheetah
 *
 * This implements the COM interface for the finder plugin
 *
 * Copyright (C) Heiko Voigt, 2009
 *
 * inspired by an example from Brent Simmons
 * brent@ranchero.com, http://macte.ch/kmyXM
 *
 */

#include "../common/git-compat-util.h"
#include "../common/strbuf.h"
#include "../common/debug.h"
#include "../common/menuengine.h"
#include "../common/cheetahmenu.h"
#include "util.h"
#include "menu.h"
#include "plugin.h"

void *create_instance(CFAllocatorRef a, CFUUIDRef id);
static HRESULT query_interface(void *me, REFIID id, LPVOID *ret);
static ULONG retain(void *me);
static ULONG release(void *me);

static ContextualMenuInterfaceStruct plugin_virtual_table = {
	NULL,
	/* Standard COM */
	query_interface,
	retain,
	release,
	/* Context Menu Interface */
	query_context_menu,
	invoke_command,
	cleanup_context_menu
};

void *create_instance(CFAllocatorRef a, CFUUIDRef id)
{
	struct plugin_data *me;

	if (!CFEqual(id, kContextualMenuTypeID))
		return NULL;

	CFPlugInAddInstanceForFactory(myUUID);

	me = (struct plugin_data *) malloc(sizeof(struct plugin_data));
	memset(me, 0, sizeof(struct plugin_data));

	me->virtual_table = &plugin_virtual_table;
	me->id = CFRetain(myUUID);
	me->num_ref = 1;

	return me;
}

static HRESULT query_interface(void *_me, REFIID id, LPVOID *ret)
{
	struct plugin_data *me = _me;
	Boolean interface_supported = false;
	CFUUIDRef query_id = CFUUIDCreateFromUUIDBytes(NULL, id);

	if (CFEqual(query_id, kContextualMenuInterfaceID))
		interface_supported = true;

	if (CFEqual(query_id, IUnknownUUID))
		interface_supported = true;

	CFRelease(query_id);

	if (!interface_supported) {
		*ret = NULL;
		return E_NOINTERFACE;
	}

	retain(me);
	*ret = me;

	return S_OK;
}

static ULONG retain(void *_me)
{
	struct plugin_data *me = _me;
	return ++(me->num_ref);
}

static ULONG release(void *_me)
{
	struct plugin_data *me = _me;
	me->num_ref--;

	if(me->num_ref > 0)
		return me->num_ref;

	if(me->id) {
		CFPlugInRemoveInstanceForFactory(me->id);
		CFRelease(me->id);
	}

	free(me);
	return 0;
}
