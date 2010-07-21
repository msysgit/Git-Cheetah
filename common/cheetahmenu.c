
#include "cache.h"
#include "exec.h"
#include "menuengine.h"
#include "cheetahmenu.h"
#include "debug.h"
#include "systeminfo.h"

char *wd_from_path(const char *path, BOOL *is_path_dir)
{
	BOOL directory = TRUE;
	char *cheetah_wd = strdup(path);
	if (!is_directory(cheetah_wd)) {
		char *c = strrchr(cheetah_wd, PATH_SEPERATOR);
		if (c) /* sanity check in case it's a weird directory */
			*c = 0;

		directory = FALSE;
	}

	if (is_path_dir)
		*is_path_dir = directory;

	return cheetah_wd;
}

/*
 * Cheetah-specific menu
 */

static int menu_gui(struct git_data *this_, UINT id)
{
	char *wd = wd_from_path(this_->name, NULL);
	const char **argv;

	free_func_t argv_free;
	void *argv_data;

	const char *generic_argv[] = { "git", "gui", NULL };

	argv = menu_get_platform_argv(MENU_GUI, NULL,
			&argv_free, &argv_data);

	if (!argv)
		argv = generic_argv;

	exec_program_v(wd, NULL, NULL, HIDDENMODE, argv);

	if (argv_free)
		argv_free(argv_data);
	free(wd);

	return 0;
}

static int menu_init(struct git_data *this_, UINT id)
{
	char *wd = wd_from_path(this_->name, NULL);
	const char **argv;

	free_func_t argv_free;
	void *argv_data;

	const char *generic_argv[] = { "git", "init", NULL };

	argv = menu_get_platform_argv(MENU_INIT, NULL,
			&argv_free, &argv_data);
	if (!argv)
		argv = generic_argv;

	exec_program_v(wd, NULL, NULL, HIDDENMODE | WAITMODE, argv);

	if (argv_free)
		argv_free(argv_data);
	free(wd);

	return 1;
}

static int menu_history(struct git_data *this_, unsigned int id)
{
	BOOL is_directory;
	char *wd = wd_from_path(this_->name, &is_directory);
	char *name = "";
	const char **argv;

	free_func_t argv_free;
	void *argv_data;

	const char *generic_argv[] = { "gitk", "HEAD", "--",
		NULL, NULL };

	if (!is_directory)
		name = this_->name + strlen(wd) + 1;
	generic_argv[3] = name;

	argv = menu_get_platform_argv(MENU_HISTORY, name,
			&argv_free, &argv_data);
	if (!argv)
		argv = generic_argv;

	exec_program_v(wd, NULL, NULL, HIDDENMODE, argv);

	if (argv_free)
		argv_free(argv_data);
	free(wd);

	return 0;
}

static int menu_bash(struct git_data *this_, UINT id)
{
	char *wd = wd_from_path(this_->name, NULL);
	const char **argv;

	free_func_t argv_free;
	void *argv_data;

	argv = menu_get_platform_argv(MENU_BASH, wd,
			&argv_free, &argv_data);
	/* There is no generic implementation for this item */
	if (!argv) {
		debug_git("Error: Got no platform terminal for bash");
		return 0;
	}

	exec_program_v(wd, NULL, NULL, NORMALMODE, argv);

	if (argv_free)
		argv_free(argv_data);
	free(wd);

	return 0;
}

static int menu_blame(struct git_data *this_, UINT id)
{
	BOOL is_directory;
	char *wd = wd_from_path(this_->name, &is_directory);
	char *name = "";
	const char **argv;

	free_func_t argv_free = NULL;
	void *argv_data;

	const char *generic_argv[] = { "git", "gui", "blame",
		NULL, NULL };

	if (!is_directory) {
		name = this_->name + strlen(wd) + 1;
		generic_argv[3] = name;

		argv = menu_get_platform_argv(MENU_BLAME, name,
				&argv_free, &argv_data);
		if (!argv)
			argv = generic_argv;

		exec_program_v(wd, NULL, NULL, HIDDENMODE, argv);
	}

	if (argv_free)
		argv_free(argv_data);
	free(wd);

	return 0;
}

static int menu_citool(struct git_data *this_, UINT id)
{
	char *wd = wd_from_path(this_->name, NULL);
	const char **argv;

	free_func_t argv_free;
	void *argv_data;

	const char *generic_argv[] = { "git", "citool", NULL };

	argv = menu_get_platform_argv(MENU_CITOOL, NULL,
			&argv_free, &argv_data);
	if (!argv)
		argv = generic_argv;

	exec_program_v(wd, NULL, NULL, HIDDENMODE, argv);

	if (argv_free)
		argv_free(argv_data);
	free(wd);

	return 0;
}

static int menu_addall(struct git_data *this_, UINT id)
{
	char *wd = wd_from_path(this_->name, NULL);
	const char **argv;

	free_func_t argv_free;
	void *argv_data;

	const char *generic_argv[] = { "git", "add", "--all", NULL };

	argv = menu_get_platform_argv(MENU_ADDALL, NULL,
			&argv_free, &argv_data);
	if (!argv)
		argv = generic_argv;

	exec_program_v(wd, NULL, NULL, HIDDENMODE, argv);

	if (argv_free)
		argv_free(argv_data);
	free(wd);

	return 0;
}

static int menu_branch(struct git_data *this_, UINT id)
{
	int status;
	char *wd = wd_from_path(this_->name, NULL);
	struct strbuf err;
	const char *menu_item_text;
	const char **argv;

	free_func_t argv_free;
	void *argv_data;

	const char *generic_argv[] = { "git", "checkout", NULL, NULL };

	menu_item_text = get_menu_item_text(id);
	generic_argv[2] = menu_item_text;

	argv = menu_get_platform_argv(MENU_BRANCH, menu_item_text,
			&argv_free, &argv_data);
	if (!argv)
		argv = generic_argv;

	strbuf_init(&err, 0);

	status = exec_program_v(wd, NULL, &err, HIDDENMODE | WAITMODE, argv);

	/* if nothing, terribly wrong happened, show the confirmation */
	if (-1 != status)
		/* strangely enough even success message is on stderr */
		debug_git_mbox(err.buf);

	if (argv_free)
		argv_free(argv_data);
	free(wd);

	return 1;
}

static BOOL build_branch_menu(struct git_data *data,
			      const struct menu_item *item,
			      void *platform)
{
	void *submenu;

	int status;
	char *wd = wd_from_path(data->name, NULL);

	struct strbuf output;
	struct strbuf **lines, **it;
	strbuf_init(&output, 0);

	status = exec_program(wd, &output, NULL, WAITMODE,
		"git", "branch", NULL);
	free(wd);
	if (status)
		return FALSE;

	submenu = start_submenu(data, item, platform);

	lines = strbuf_split(&output, '\n');
	for (it = lines; *it; it++) {
		struct menu_item item = {
			MENU_ITEM_CLEANUP, 0,
			NULL, NULL,
			NULL, menu_branch
		};

		strbuf_rtrim(*it);
		item.string = strdup((*it)->buf + 2);
		item.helptext = strdup((*it)->buf + 2);
		item.flags = '*' == (*it)->buf[0] ? MI_CHECKED : 0;
		if (build_item(data, &item, submenu))
			append_active_menu(&item);
		else
			/*
			 * if the platform failed to create an item
			 * there is no point to try other items
			 */
			break;
	}

	end_submenu(platform, submenu);

	/* technically, there is nothing to track for the menu engine */
	return FALSE;
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
	{ MENU_ITEM_ALWAYS, 0, NULL, NULL, build_separator, NULL },

	{ MENU_ITEM_REPO, 0, "Git &Add all files now",
		"Add all files from this folder now",
		build_item, menu_addall },
	{ MENU_ITEM_REPO, 0, "Git &Commit Tool",
		"Launch the GIT commit tool in the local or chosen directory.",
		build_item, menu_citool },
	{ MENU_ITEM_TRACK, 0, "Git &History",
		"Show GIT history of the chosen file or directory.",
		build_item,
		menu_history },
	{ MENU_ITEM_TRACK | MENU_ITEM_FILE, 0, "Git &Blame",
		"Start a blame viewer on the specified file.",
		build_item, menu_blame },

	{ MENU_ITEM_REPO, 0, "Git &Gui",
		"Launch the GIT Gui in the local or chosen directory.",
		build_item, menu_gui },

	{ MENU_ITEM_REPO, 0, "Git Bra&nch",
		"Checkout a branch",
		build_branch_menu, NULL },

	{ MENU_ITEM_NOREPO, 0, "Git I&nit Here",
		"Initialize GIT repo in the local directory.",
		build_item, menu_init },
	{ MENU_ITEM_NOREPO | MENU_ITEM_DIR, 0, "Git &Gui",
		"Launch the GIT Gui in the local or chosen directory.",
		build_item, menu_gui },

	{ MENU_ITEM_ALWAYS, 0, "Git Ba&sh",
		"Start GIT shell in the local or chosen directory",
		build_item, menu_bash },
	{ MENU_ITEM_ALWAYS, 0, NULL, NULL, build_separator, NULL },
};

void build_cheetah_menu(struct git_data *data, void *platform_data)
{
	reset_platform(platform_data);
	build_menu_items(data, cheetah_menu_mask,
		cheetah_menu,
		sizeof(cheetah_menu) / sizeof(cheetah_menu[0]),
		platform_data);
}
