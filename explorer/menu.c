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
	windows_menu->index++;

	return TRUE;
}

void *start_submenu(struct git_data *this_, const struct menu_item *item,
		    void *platform)
{
	struct windows_menu_data *parent_menu = platform;
	struct windows_menu_data *submenu =
		malloc(sizeof(struct windows_menu_data));
	submenu->menu = CreateMenu();
	InsertMenu(parent_menu->menu, parent_menu->index,
		MF_POPUP | MF_BYPOSITION, (UINT_PTR)(submenu->menu),
		item->string);
	parent_menu->index++;

	submenu->index = 0;
	submenu->first = parent_menu->first;

	return submenu;
}

void end_submenu(void *parent, void *submenu)
{
	free(submenu);
}

void check_menu_item(void *platform, int checked)
{
	struct windows_menu_data *submenu = platform;
	/* -1, because it's called __after__ index is increased */
	CheckMenuItem(submenu->menu, submenu->index - 1,
		MF_BYPOSITION | (checked ? MF_CHECKED : 0));
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
	int i;
	int size_incr = 0;
	char *converted;
	char *dst;

	/* Figure out how much extra space we need to escape spaces */
	for (i = 0; i < MAX_PATH && path[i] != '\0'; ++i)
		if (path[i] == ' ')
			size_incr++;

	converted = (char *)calloc(size_incr + i + 1, sizeof(char));
	dst = converted;

	/* Transform:
	 * " " -> "\ "
	 * "\" -> "/"
	 */
	for (i = 0; i < MAX_PATH && path[i] != '\0'; ++i)
	{
		switch (path[i])
		{
		case ' ':
			*(dst++) = '\\';
			*(dst++) = ' ';
			break;
		case '\\':
			*(dst++) = '/';
			break;
		default:
			*(dst++) = path[i];
			break;
		}
	}
	*dst = '\0';

	/* X: -> /X */
	converted[1] = converted[0];
	converted[0] = '/';

	return converted;
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
		LPWSTR tw = malloc(len * sizeof(wchar_t));
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
