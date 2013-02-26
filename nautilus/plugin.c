/* plugin.c
 *
 * This file implements the interface of the Gnome filemanager Nautilus.
 * It connects the interface with the implementation of the platform
 * specific functions found in menu.c for git-cheetah.
 *
 * Copyright (C) Heiko Voigt, 2009
 *
 */

#include "../common/git-compat-util.h"
#include "../common/debug.h"
#include "../common/strbuf.h"
#include "../common/menuengine.h"
#include "../common/cheetahmenu.h"

#include "plugin.h"
#include "menu.h"


/* declarations */

typedef struct {
	GObject parent_slot;
} GitExtension;

typedef struct {
	GObjectClass parent_slot;
} GitExtensionClass;

void nautilus_module_initialize(GTypeModule  *module);
void nautilus_module_shutdown(void);
void nautilus_module_list_types(const GType **types, int *num_types);
GType git_extension_get_type(void);

static void git_extension_register_type(GTypeModule *module);
static void git_extension_menu_provider_iface_init(
		NautilusMenuProviderIface *iface);


/* implementations */

static GType provider_types[1];
static GType git_extension_type;
static GObjectClass *parent_class;

void nautilus_module_initialize(GTypeModule  *module)
{
	git_extension_register_type(module);

	provider_types[0] = git_extension_get_type();
}

void nautilus_module_shutdown(void)
{
	/* Any module-specific shutdown */
}

void nautilus_module_list_types(const GType **types,
		int *num_types)
{
	*types = provider_types;
	*num_types = G_N_ELEMENTS(provider_types);
}

GType git_extension_get_type(void)
{
	return git_extension_type;
}

static void git_extension_instance_init(GitExtension *object)
{
}

static void git_extension_class_init(GitExtensionClass *class)
{
	parent_class = g_type_class_peek_parent(class);
}

static void git_extension_register_type(GTypeModule *module)
{
	static const GTypeInfo info = {
		sizeof(GitExtensionClass),
		(GBaseInitFunc) NULL,
		(GBaseFinalizeFunc) NULL,
		(GClassInitFunc) git_extension_class_init,
		NULL,
		NULL,
		sizeof(GitExtension),
		0,
		(GInstanceInitFunc) git_extension_instance_init,
	};

	static const GInterfaceInfo menu_provider_iface_info = {
		(GInterfaceInitFunc) git_extension_menu_provider_iface_init,
		NULL,
		NULL
	};

	git_extension_type = g_type_module_register_type(module,
			G_TYPE_OBJECT,
			"GitExtension",
			&info, 0);

	g_type_module_add_interface(module,
			git_extension_type,
			NAUTILUS_TYPE_MENU_PROVIDER,
			&menu_provider_iface_info);
}

static void git_extension_menu_provider_iface_init(
		NautilusMenuProviderIface *iface)
{
	iface->get_file_items = git_extension_get_file_items;
	iface->get_background_items = git_extension_get_background_items;
}
