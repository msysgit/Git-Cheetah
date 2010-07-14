/* menu.c
 *
 * Here we implement the functions which feed the cheetah menuengine
 * for Nautilus.
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

static struct git_data git_data;

BOOL build_item(struct git_data *me, const struct menu_item *item,
		void *platform)
{
	char *item_name;
	char shortcut_key;
	char item_reference_string[1024];
	NautilusMenuItem *one_menu_item;
	struct nautilus_menu_data *nautilus_data = platform;
	static int not_shown_offset = 0;

	if (!platform) {
		not_shown_offset++;
		return TRUE;
	}

	if (not_shown_offset) {
		nautilus_data->item_id += not_shown_offset;
		not_shown_offset = 0;
	}

	item_name = strdup(item->string);

	shortcut_key = parse_and_remove_shortcuts(item_name);

	debug_git("Adding entry: %s", item_name);

	snprintf(item_reference_string, 1024, "GitExtension::%s", item_name);
	item_reference_string[1023] = '\0';
	one_menu_item = nautilus_menu_item_new(item_reference_string,
			item_name,
			item->helptext,
			NULL /* icon name */);
	g_signal_connect(one_menu_item, "activate",
			G_CALLBACK(invoke_command), nautilus_data->provider);

	g_object_set_data((GObject *) one_menu_item, "git_extension_id",
			(void *) nautilus_data->item_id);
	g_object_set_data((GObject *) one_menu_item,
			"git_extension_git_data", me);

	nautilus_data->menu_items = g_list_append(nautilus_data->menu_items,
			one_menu_item);

	/* this needs to be uniqe for each item */
	nautilus_data->item_id++;

	free(item_name);
	return TRUE;
}

BOOL build_separator(struct git_data *me, const struct menu_item *item,
		void *platform)
{
	/* not implemented, yet */
	struct nautilus_menu_data *nautilus_data = platform;
	nautilus_data->item_id++;
	return TRUE;
}

void reset_platform(void *platform)
{
	/* nautilus does not need to reset anything */
}

void *start_submenu(struct git_data *me, const struct menu_item *item,
		void *platform)
{
	/* not implemented, yet */
	return NULL;
}

void end_submenu(void *parent, void *submenu)
{
	/* not implemented, yet */
}

static inline char *get_local_filename_from_fileinfo(char *filename, int n,
		NautilusFileInfo *file)
{
	char *name;
	char locator[MAX_PATH];

	name = nautilus_file_info_get_uri(file);

	/* find out if this is a local file */
	strncpy(locator, name, MAX_PATH);
	locator[7] = '\0';
	if (!strcmp(locator, "file://")) {
		strncpy(filename, name+7, n);
		filename[n-1] = '\0';
	}
	else
		filename=NULL;

	g_free(name);
	return filename;
}

GList *git_extension_get_file_items(NautilusMenuProvider *provider,
		GtkWidget *window,
		GList *files)
{
	GList *l;

	if (!files)
		return NULL;

	struct nautilus_menu_data nautilus_data = {
		provider,
		NULL,
		0
	};


	git_data.name[0] = '\0';
	for (l = files; l != NULL; l = l->next) {
		get_local_filename_from_fileinfo(git_data.name,
				MAX_PATH, NAUTILUS_FILE_INFO(l->data));
	}

	/* if we could not get a local file we stop */
	if (!git_data.name[0])
		return NULL;

	debug_git("selected %s\n", git_data.name);

	build_cheetah_menu(&git_data, &nautilus_data);

	return nautilus_data.menu_items;
}

GList *git_extension_get_background_items(NautilusMenuProvider *provider,
		GtkWidget *window,
		NautilusFileInfo *current_folder)
{
	if (!current_folder)
		return NULL;

	struct nautilus_menu_data nautilus_data = {
		provider,
		NULL,
		0
	};

	git_data.name[0] = '\0';
	get_local_filename_from_fileinfo(git_data.name,
			MAX_PATH, NAUTILUS_FILE_INFO(current_folder));

	/* if we could not get a local file we stop */
	if (!git_data.name[0])
		return NULL;

	debug_git("selected %s\n", git_data.name);

	build_cheetah_menu(&git_data, &nautilus_data);

	return nautilus_data.menu_items;
}

GList *git_extension_get_toolbar_items(NautilusMenuProvider *provider,
		GtkWidget *window,
		NautilusFileInfo *current_folder)
{
	return git_extension_get_background_items(provider,
			window, current_folder);
}

void invoke_command(NautilusMenuItem *item,
		gpointer user_data)
{
	int id;
	struct git_data *me;

	id = (int) g_object_get_data((GObject *) item, "git_extension_id");
	me = (struct git_data *) g_object_get_data((GObject *) item,
			"git_extension_git_data");

	debug_git("invoke_command: %s, id: %i", me->name, id);

	handle_menu_item(me, id);
}

const char **menu_get_platform_argv(menu_commands cmd, const void *data,
		free_func_t *free_argv, void **argv_data)
{
	static const char *bash_argv[] = { "gnome-terminal", NULL };

	*free_argv = NULL;
	*argv_data = NULL;

	switch(cmd)
	{
		case MENU_BASH:
			return bash_argv;
		default:
			return NULL;
	}
}
