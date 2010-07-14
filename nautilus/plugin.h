#ifndef NAUTILUS_PLUGIN_H
#define NAUTILUS_PLUGIN_H

#include <glib.h>
/* Nautilus extension headers */
#include <libnautilus-extension/nautilus-extension-types.h>
#include <libnautilus-extension/nautilus-file-info.h>
#include <libnautilus-extension/nautilus-info-provider.h>
#include <libnautilus-extension/nautilus-menu-provider.h>
#include <libnautilus-extension/nautilus-property-page-provider.h>


#include <string.h>
#include <time.h>

struct nautilus_menu_data {
	NautilusMenuProvider *provider;
	GList *menu_items;
	NautilusMenu *submenu;
};

#endif /* NAUTILUS_PLUGIN_H */
