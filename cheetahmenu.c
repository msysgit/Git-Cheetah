
#include "cache.h"
#include "exec.h"
#include "menuengine.h"
#include "cheetahmenu.h"

char *wd_from_path(const char *path, BOOL *is_path_dir)
{
	BOOL is_directory = TRUE;
	char *cheetah_wd = strdup(path);
	if (!(FILE_ATTRIBUTE_DIRECTORY & GetFileAttributes(cheetah_wd))) {
		char *c = strrchr(cheetah_wd, '\\');
		if (c) /* sanity check in case it's a weird directory */
			*c = 0;

		is_directory = FALSE;
	}

	if (is_path_dir)
		*is_path_dir = is_directory;

	return cheetah_wd;
}

/*
 * Cheetah-specific menu
 */

static void menu_gui(struct git_data *this_, UINT id)
{
	char *wd = wd_from_path(this_->name, NULL);
	exec_program(wd, NULL, NULL, HIDDENMODE, "git", "gui", NULL);
	free(wd);
}

static void menu_history(struct git_data *this_, unsigned int id)
{
	BOOL is_directory;
	char *wd = wd_from_path(this_->name, &is_directory);
	char *name = "";

	if (!is_directory)
		name = this_->name + strlen(wd) + 1;

	exec_program(wd, NULL, NULL, HIDDENMODE, "sh", "--login", "-i",
		"/bin/gitk", "HEAD", "--", name, NULL);
	free(wd);
}

static void menu_bash(struct git_data *this_, UINT id)
{
	char *wd = wd_from_path(this_->name, NULL);
	/* start is required because exec_program does not create a window */
	exec_program(wd, NULL, NULL, NORMALMODE,
		"start", "sh", "--login", "-i", NULL);
	free(wd);
}

static void menu_blame(struct git_data *this_, UINT id)
{
	BOOL is_directory;
	char *wd = wd_from_path(this_->name, &is_directory);
	char *name = "";

	if (!is_directory) {
		name = this_->name + strlen(wd) + 1;
		exec_program(wd, NULL, NULL, HIDDENMODE,
			"git", "gui", "blame", name, NULL);
	}

	free(wd);
}

UINT cheetah_menu_mask(struct git_data *this_)
{
	BOOL is_directory;
	char *wd = wd_from_path(this_->name, &is_directory);
	UINT selection = is_directory ? MENU_ITEM_DIR : MENU_ITEM_FILE;
	int status;

	struct strbuf output;
	char *eol;
	strbuf_init(&output, 0);

	status = exec_program(wd, &output, NULL, WAITMODE,
		"git", "rev-parse", "--show-prefix", NULL);
	eol = strchr(output.buf, '\n');
	if (eol)
		*eol = 0;

	if (status < 0) /* something went terribly wrong */
		selection = MENU_ITEM_LAST;
	else if (status)
		selection |= MENU_ITEM_NOREPO;
	else {
		char head_path[MAX_PATH] = "HEAD";
		if (!is_directory)
			sprintf(head_path, "HEAD:%s%s",
				output.buf,
				this_->name + strlen(wd) + 1);

		status = exec_program(wd, NULL, NULL, WAITMODE,
			"git", "rev-parse", "--verify", head_path, NULL);
		if (status < 0) /* something went terribly wrong */
			selection = MENU_ITEM_LAST;
		else
			selection |= MENU_ITEM_REPO |
				(status ?
					MENU_ITEM_NOTRACK : MENU_ITEM_TRACK);
	}

	strbuf_release(&output);
	free(wd);
	return selection;
}

const struct menu_item cheetah_menu[] = {
	{ MENU_ITEM_ALWAYS, NULL, NULL, build_separator, NULL },

	{ MENU_ITEM_TRACK, "Git &History",
		"Show GIT history of the chosen file or directory.",
		build_item,
		menu_history },
	{ MENU_ITEM_TRACK | MENU_ITEM_FILE, "Git &Blame",
		"Start a blame viewer on the specified file.",
		build_item, menu_blame },

	{ MENU_ITEM_REPO, "&Git",
		"Launch the GIT Gui in the local or chosen directory.",
		build_item, menu_gui },

	{ MENU_ITEM_NOREPO | MENU_ITEM_FILE, "&Git Init Here",
		"Initialize GIT repo in the local directory.",
		build_item, menu_gui },
	{ MENU_ITEM_NOREPO | MENU_ITEM_DIR, "&Git Clone Here",
		"Clone GIT repo into the local or chosen directory.",
		build_item, menu_gui },

	{ MENU_ITEM_ALWAYS, "Git Ba&sh",
		"Start GIT shell in the local or chosen directory",
		build_item, menu_bash },
};

void build_cheetah_menu(struct git_data *data, void *platform_data)
{
	reset_platform(platform_data);
	build_menu_items(data, cheetah_menu_mask,
		cheetah_menu,
		sizeof(cheetah_menu) / sizeof(cheetah_menu[0]),
		platform_data);
}
