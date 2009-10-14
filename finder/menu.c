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
#include "../compat/util.h"
#include "plugin.h"
#include "util.h"
#include "menu.h"

BOOL build_item(struct git_data *me, const struct menu_item *item,
		void *platform)
{
	struct osx_menu_data *osx_data = platform;
	AERecord menu_entry = { typeNull, NULL };
	BOOL status = TRUE;
	char *item_name = strdup(item->string);
	char shortcut_key;
	static int not_shown_offset = 0;

	if (!platform) {
		not_shown_offset++;
		return TRUE;
	}

	if (not_shown_offset) {
		osx_data->item_id += not_shown_offset;
		not_shown_offset = 0;
	}

	shortcut_key = parse_and_remove_shortcuts(item_name);

	debug_git("Adding entry: %s", item_name);

	if(AECreateList(NULL, 0, true, &menu_entry) != noErr)
		return FALSE;

	if(AEPutKeyPtr(&menu_entry, keyAEName, typeCString, item_name,
				strlen(item_name) + 1) != noErr)
	{
		status = FALSE;
		goto add_menu_entry_cleanup;
	}

	if(AEPutKeyPtr(&menu_entry, 'cmcd', typeSInt32, &osx_data->item_id,
				sizeof(osx_data->item_id)) != noErr)
	{
		status = FALSE;
		goto add_menu_entry_cleanup;
	}

	/* insert menu item at the end of the menu */
	if(AEPutDesc(osx_data->menu, 0, &menu_entry) != noErr)
	{
		status = FALSE;
		goto add_menu_entry_cleanup;
	}

	/* this needs to be uniqe for each item */
	osx_data->item_id++;

add_menu_entry_cleanup:
	AEDisposeDesc(&menu_entry);
	free(item_name);
	return status;
}

BOOL build_separator(struct git_data *me, const struct menu_item *item,
		void *platform)
{
	struct osx_menu_data *osx_data = platform;
	osx_data->item_id++;
	return TRUE;
}

/* osx does not need to reset anything */
void reset_platform(void *platform)
{
}

void *start_submenu(struct git_data *me, const struct menu_item *item,
		void *platform)
{
	/* not implemented, yet */
	return NULL;
}

void end_submenu(void *parent, void *submenu)
{
}

void check_menu_item(void *platform, int checked)
{
}

OSStatus query_context_menu(void *_me, const AEDescList *selection,
		AEDescList *menu)
{
	struct plugin_data *me = _me;
	struct osx_menu_data osx_data = { menu, selection, noErr , 0 };

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

const char **menu_get_platform_argv(menu_commands cmd, const void *data)
{
	int n;
	const char *wd;
	static char apple_script[MAX_PATH+32];
	static const char *bash_argv[] = { "osascript", "-e",
		"tell application \"Terminal\"", "-e",
		apple_script, "-e", "end tell" };

	switch(cmd)
	{
		case MENU_BASH:
			wd = (const char *) data;
			n = snprintf(apple_script, 1024,
				"do script \"cd %s\"", wd);
			if(n > 1023)
				return NULL;

			return bash_argv;

		default:
			return NULL;
	}
}

/* this is called by the finder after clicking on some item from us */
OSStatus invoke_command(void *_me, AEDesc *selection,
		SInt32 id)
{
	struct plugin_data *me = _me;
	char path[MAX_PATH];
	if(!selection_to_path(path, MAX_PATH, selection))
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

