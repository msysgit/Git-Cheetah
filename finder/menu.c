/* menu.c
 *
 * Implementation of a finder menu plugin for git-cheetah
 *
 * This implements the cheetah interface to create menu entries.
 *
 * Copyright (C) Heiko Voigt, 2009
 *
 * inspired by an example from Brent Simmons
 * brent@ranchero.com, http://macte.ch/kmyXM
 *
 */


/* implements platform dependent functions declared by cheetah */
#include "../common/git-compat-util.h"
#include "../common/strbuf.h"
#include "../common/exec.h"
#include "../common/menuengine.h"
#include "../common/cheetahmenu.h"
#include "../common/debug.h"
#include "plugin.h"
#include "util.h"
#include "menu.h"

BOOL build_item(struct git_data *me, const struct menu_item *item,
		void *platform)
{
	struct osx_menu_data *osx_data = platform;
	AERecord menu_entry = { typeNull, NULL };
	MenuItemAttributes attr = item->flags & MI_DISABLED ?
	    kMenuItemAttrSectionHeader : 0;
	BOOL status = TRUE;
	char *item_name = strdup(item->string);
	char shortcut_key;

	if (!platform)
		return TRUE;

	shortcut_key = parse_and_remove_shortcuts(item_name);

	debug_git("Adding entry: %s", item_name);

	if ((AECreateList(NULL, 0, true, &menu_entry) != noErr) ||
	    (AEPutKeyPtr(&menu_entry, keyAEName, typeCString, item_name,
	        strlen(item_name) + 1) != noErr) ||
	    (AEPutKeyPtr(&menu_entry, keyContextualMenuCommandID, typeSInt32,
	        &next_active_item, sizeof(next_active_item)) != noErr) ||
	    (AEPutKeyPtr(&menu_entry, keyContextualMenuAttributes, typeSInt32,
	        &attr, sizeof(attr)) != noErr) ||
	    (AEPutDesc(osx_data->menu, 0, &menu_entry) != noErr))
		status = FALSE;

	AEDisposeDesc(&menu_entry);
	free(item_name);
	return status;
}

BOOL build_separator(struct git_data *me, const struct menu_item *item,
		void *platform)
{
	return TRUE;
}

/* osx does not need to reset anything */
void reset_platform(void *platform)
{
}

void *start_submenu(struct git_data *me, const struct menu_item *item,
		void *platform)
{
	struct osx_menu_data *parent_menu = platform;
	struct osx_menu_data *submenu =
	    xmalloc(sizeof(struct osx_menu_data));

	submenu->selection = parent_menu->selection;
	submenu->status = parent_menu->status;

	submenu->menu = xmalloc(sizeof(*submenu->menu));
	submenu->menu->descriptorType = typeNull;
	submenu->menu->dataHandle = NULL;
	AECreateList(NULL, 0, false, submenu->menu);

	submenu->menu_name = strdup(item->string);
	parse_and_remove_shortcuts(submenu->menu_name);
	return submenu;
}

void end_submenu(void *parent, void *platform)
{
	struct osx_menu_data *parent_menu = parent;
	struct osx_menu_data *submenu = platform;
	AERecord menu_entry = { typeNull, NULL };

	debug_git("Adding submenu: %s", submenu->menu_name);

	AECreateList(NULL, 0, true, &menu_entry);
	AEPutKeyPtr(&menu_entry, keyAEName, typeCString, submenu->menu_name,
	    strlen(submenu->menu_name) + 1);
	AEPutKeyDesc(&menu_entry, keyContextualMenuSubmenu,
	    submenu->menu);
	AEPutDesc(parent_menu->menu, 0, &menu_entry);

	AEDisposeDesc(submenu->menu);
	AEDisposeDesc(&menu_entry);
	free(submenu->menu_name);
	free(submenu->menu);
	free(submenu);
}

OSStatus query_context_menu(void *_me, const AEDescList *selection,
		AEDescList *menu)
{
	struct plugin_data *me = _me;
	struct osx_menu_data osx_data = { menu, NULL, selection, noErr };

	/* currently this fails when multiple files/directories are
	 * selected
	 *
	 * TODO: add support to handle selection of multiple items
	 * e.g. selection_to_path could be called more often until no
	 * item is left
	 */
	if (!selection_to_path(me->git_data.name, MAX_PATH, selection))
		return noErr;

	debug_git("Selected: %s", me->git_data.name);

	build_cheetah_menu(&me->git_data, &osx_data);

	return osx_data.status;
}

struct menu_argv_data {
	char *apple_script;
	const char **argv;
};

static void free_platform_argv(void *data)
{
	struct menu_argv_data *my_data = data;

	free(my_data->argv);
	free(my_data->apple_script);
	free(my_data);
}

const char **menu_get_platform_argv(menu_commands cmd, void *data,
		free_func_t *free_argv, void **argv_data)
{
	struct menu_argv_data *my_data;
	int script_len;
	const char *wd;
	char *apple_script;
	const char **argv = NULL;
	const char *bash_argv[] = { "osascript", "-e",
		"tell application \"Terminal\"", "-e",
		NULL, "-e", "end tell", NULL };

	*argv_data = NULL;
	*free_argv = NULL;

	switch(cmd)
	{
		case MENU_BASH:
			wd = (const char *) data;

			my_data = xmalloc(sizeof(struct menu_argv_data));
			apple_script = xmalloc((MAX_PATH+32) * sizeof(char));
			argv = xmalloc(sizeof(bash_argv));

			bash_argv[4] = apple_script;
			my_data->apple_script = apple_script;
			my_data->argv = argv;

			memcpy(argv, bash_argv, sizeof(bash_argv));

			script_len = snprintf(apple_script, MAX_PATH+32,
				"do script \"cd %s\"", wd);
			if (script_len >= (MAX_PATH+32))
				argv = NULL;

			*argv_data = my_data;
			*free_argv = free_platform_argv;

			break;

		default:
			return NULL;
	}

	return argv;
}

/* this is called by the finder after clicking on some item from us */
OSStatus invoke_command(void *_me, AEDesc *selection,
		SInt32 id)
{
	struct plugin_data *me = _me;
	char path[MAX_PATH];
	if (!selection_to_path(path, MAX_PATH, selection))
		return noErr;

	debug_git("invoke_command: %s, id: %i", path, id);
	handle_menu_item(&me->git_data, id);

	return noErr;
}

/* this is called after display of the menu to give the plugin a chance
 * to cleanup anything needed during display phase
 */
void cleanup_context_menu(void *_me) {
	/* we do not need to cleanup anything */
}

