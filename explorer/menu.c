#include "../common/cache.h"

#include <shlobj.h>
#include <tchar.h>
#include "../common/menuengine.h"
#include "../common/cheetahmenu.h"
#include "menu.h"
#include "ext.h"
#include "../common/debug.h"
#include "../common/systeminfo.h"
#include "../common/exec.h"

#define LONGEST_MENU_ITEM 40

/*
 * Windows-specific Cheetah menu functions
 */
struct windows_menu_data {
	HMENU menu;
	UINT index;
	UINT first;
	UINT last;
};

void reset_platform(void *platform)
{
	/* On Windows, we don't do anything to reset the menu */
}

/*
 * menu_item_builder to build a Windows-specific menu separator
 *
 * Always returns FALSE so the menu engine does not track this item
 */
BOOL build_separator(struct git_data *data, const struct menu_item *item,
		     void *platform)
{
	struct windows_menu_data *windows_menu = platform;
	InsertMenu(windows_menu->menu, windows_menu->index,
		MF_SEPARATOR | MF_BYPOSITION, 0, "");
	windows_menu->index++;

	return FALSE;
}

/*
 * menu_item_builder to build a simple menu item
 *
 * Explorer's context menu are limited in the number of comands
 * that they can use, so build_item would:
 * - do nothing if that limit is reached and return FALSE to
 *   instruct the menu engine to not track this item
 * - create item and return TRUE, so the item can be handled later
 */
BOOL build_item(struct git_data *data, const struct menu_item *item,
		void *platform)
{
	struct windows_menu_data *windows_menu = platform;
	if (windows_menu->last < windows_menu->first + next_active_item)
		return FALSE;

	InsertMenu(windows_menu->menu, windows_menu->index,
		MF_STRING | MF_BYPOSITION,
		windows_menu->first + next_active_item,
		item->string);

	if (item->flags & MI_CHECKED)
		CheckMenuItem(windows_menu->menu, windows_menu->index,
			MF_BYPOSITION | MF_CHECKED);

	windows_menu->index++;
	return TRUE;
}

void *start_submenu(struct git_data *this_, const struct menu_item *item,
		    void *platform)
{
	struct windows_menu_data *parent_menu = platform;
	struct windows_menu_data *submenu =
		xmalloc(sizeof(struct windows_menu_data));
	submenu->menu = CreateMenu();
	InsertMenu(parent_menu->menu, parent_menu->index,
		MF_POPUP | MF_BYPOSITION, (UINT_PTR)(submenu->menu),
		item->string);
	parent_menu->index++;

	submenu->index = 0;
	submenu->first = parent_menu->first;
	submenu->last = parent_menu->last;

	return submenu;
}

void end_submenu(void *parent, void *submenu)
{
	free(submenu);
}

/*
 * These are the functions for handling the context menu.
 */

inline STDMETHODIMP query_context_menu(void *p, HMENU menu,
				       UINT index, UINT first_command,
				       UINT last_command, UINT flags)
{
	struct git_menu *this_menu = p;
	struct git_data *this_ = this_menu->git_data;
	struct windows_menu_data windows_menu =
		{ menu, index, first_command, last_command };

	if (flags & CMF_DEFAULTONLY)
		return MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_NULL, 0);

	build_cheetah_menu(this_, &windows_menu);

	return MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_NULL,
		next_active_item);
}

/*
 * Perform a couple of transformations, such that a directory
 *    C:\Program Files\Bunch of stuff\in\A dir
 * becomes
 *    /C/Program\ Files/Bunch\ of\ stuff/in/A\ dir
 *
 * Assumes path is initially a correctly formed Windows-style path.
 * Returns a new string.
 */
static char *convert_directory_format(const char *path)
{
	/* assuming that each character has to be escaped,
	allocate twice as much memory */
	char *converted = (char *)calloc(2 * strlen(path) + 1, sizeof(char));
	char *dst = converted;

	/* Transform:
	* "\" -> "/"
	* chars, special to bash, are escaped with "\"
	*/
	for (; *path; path++)
	{
		switch (*path)
		{
		case ' ':
		case '(':
		case ')':
		case ';':
		case '\'':
			*(dst++) = '\\';
			*(dst++) = *path;
			break;
		case '\\':
			*(dst++) = '/';
			break;
		default:
			*(dst++) = *path;
			break;
		}
	}
	*dst = '\0';

	/* X: -> /X */
	converted[1] = converted[0];
	converted[0] = '/';

	return converted;
}

static void free_platform_argv(void *data)
{
	free(data);
}

#ifndef _WIN64
#define SYSTEMDIR "system32"
#else
#define SYSTEMDIR "syswow64"
#endif

static const char *get_cmd(void)
{
	static struct strbuf buf = STRBUF_INIT;

	if (!buf.len)
		strbuf_addf(&buf, "%s\\" SYSTEMDIR "\\cmd.exe",
			getenv("WINDIR"));

	return buf.buf;
}

static void *create_bash_argv(char *wd)
{
	/* start is required because exec_program does not create a window */
	static const char *bash_argv[] = { NULL, "/c", "start",
		"sh", "-c", NULL, NULL };
	static const char *command = "cd %s && sh -l -i";
	void *argv = xmalloc(sizeof(bash_argv));
	struct strbuf shell_cmd = STRBUF_INIT;
	char *converted = convert_directory_format(wd);

	/* strbuf_addf allocates only 64 bytes, so we have to grow it manually */
	strbuf_grow(&shell_cmd, strlen(converted) + strlen(command) + 1);
	strbuf_addf(&shell_cmd, command, converted);
	free(converted);

	bash_argv[0] = get_cmd();
	bash_argv[5] = shell_cmd.buf;

	memcpy(argv, bash_argv, sizeof(bash_argv));

	/* start the cmd on a system drive, so it does not fail on UNC */
	strcpy(wd, getenv("SYSTEMDRIVE"));

	return argv;
}

static void free_bash_argv(void *data)
{
	void **argv = (void **)data;
	free(argv[5]);
	free(data);
}

const char **menu_get_platform_argv(menu_commands cmd, void *data,
		free_func_t *free_argv, void **argv_data)
{
	char *wd = data;
	const char **argv;
	const char *history_argv[] = { "sh", "--login", "-i",
		"/bin/gitk", "HEAD", "--", NULL, NULL };
	*free_argv = NULL;
	*argv_data = NULL;
	struct branch_gitk_data *branch_gitk = data;

	switch(cmd)
	{
		case MENU_HISTORY:
			history_argv[6] = wd;

			argv = xmalloc(sizeof(history_argv));
			memcpy(argv, history_argv, sizeof(history_argv));
			*free_argv = free_platform_argv;

			break;

		case MENU_BRANCH_GITK:

			history_argv[4] = branch_gitk->branch;
			history_argv[6] = branch_gitk->file;

			argv = xmalloc(sizeof(history_argv));
			memcpy(argv, history_argv, sizeof(history_argv));
			*free_argv = free_platform_argv;

			break;


		case MENU_BASH:

			argv = create_bash_argv(wd);
			*free_argv = free_bash_argv;

			break;

		default:
			return NULL;
	}

	*argv_data = argv;

	return argv;
}

inline STDMETHODIMP invoke_command(void *p,
				   LPCMINVOKECOMMANDINFO info)
{
	struct git_menu *this_menu = p;
	struct git_data *this_ = this_menu->git_data;
	UINT id = LOWORD(info->lpVerb);

	if (HIWORD(info->lpVerb))
		return E_INVALIDARG;

	handle_menu_item(this_, id);
	return S_OK;
}

inline STDMETHODIMP get_command_string(void *p, UINT id,
				       UINT flags, UINT *reserved,
				       LPSTR name, UINT size)
{
	const char *text;

	if (!(flags & GCS_HELPTEXT))
		return E_INVALIDARG;

	text = get_menu_item_text(id);
	if (!text)
		return E_INVALIDARG;

	if (flags & GCS_UNICODE) {
		size_t len = strlen(text) + 1;
		LPWSTR tw = xmalloc(len * sizeof(wchar_t));
		/* need to convert terminating NULL as well */
		mbstowcs(tw, text, len);
		lstrcpynW((LPWSTR)name, tw, size);
		free(tw);
	} else
		lstrcpynA(name, text, size);

	return S_OK;
}

DEFINE_STANDARD_METHODS(git_menu)

struct git_menu_virtual_table git_menu_virtual_table = {
	query_interface_git_menu,
	add_ref_git_menu,
	release_git_menu,
	query_context_menu,
	invoke_command,
	get_command_string
};
