#include "cache.h"

#include <shlobj.h>
#include <tchar.h>
#include "menu.h"
#include "ext.h"
#include "debug.h"
#include "systeminfo.h"
#include "exec.h"

#define LONGEST_MENU_ITEM 40

/*
 * These are the functions for handling the context menu.
 */

static STDMETHODIMP query_context_menu(void *p, HMENU menu,
				       UINT index, UINT first_command,
				       UINT last_command, UINT flags)
{
	struct git_menu *this_menu = p;
	struct git_data *this_ = this_menu->git_data;
	char *wd;
	BOOL bDirSelected = TRUE;

	UINT original_first = first_command;
	char menu_item[LONGEST_MENU_ITEM];

	int status;

	if (flags & CMF_DEFAULTONLY)
		return MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_NULL, 0);

	/* figure out the directory */
	wd = strdup(this_->name);
	if (!(FILE_ATTRIBUTE_DIRECTORY & GetFileAttributes(wd))) {
		char *c = strrchr(wd, '\\');
		if (c) {
			*c = 0;
			bDirSelected = FALSE;
		}
	}

	status = exec_program(wd, NULL, NULL, WAITMODE,
		"git", "rev-parse", "--show-cdup", NULL);
	free (wd);

	/* something really bad happened, could run git */
	if (status < 0)
		return MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_NULL, 0);

	/*
	 * TODO: the following big, ugly code needs to be something like
	 *       build_menu_items()
	 *       It's left as is to signify the preview nature of the patch
	 */
	if (status) { /* this is not a repository */
		if (bDirSelected)
			strcpy(menu_item, "&Git Clone Here");
		else
			strcpy(menu_item, "&Git Init Here");
	} else
		strcpy(menu_item, "&Git");

	InsertMenu(menu, index, MF_SEPARATOR | MF_BYPOSITION,
			first_command++, "");
	InsertMenu(menu, index+1, MF_STRING | MF_BYPOSITION,
		   first_command++, menu_item);
	
	/*
	 * TODO: when the above block is fixed, we'll just have
	 *       return MAKE_RESULT(..., build_menu_items());
	 */
	return MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_NULL,
		first_command - original_first);
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

static STDMETHODIMP invoke_command(void *p,
				   LPCMINVOKECOMMANDINFO info)
{
	struct git_menu *this_menu = p;
	struct git_data *this_ = this_menu->git_data;
	int command = LOWORD(info->lpVerb);

	if (HIWORD(info->lpVerb) != 0)
		return E_INVALIDARG;

	if (command == 1)
	{
		const char *wd;
		DWORD dwAttr, fa;

		wd = this_->name;
		if (wd == NULL || strlen(wd) == 0)
			wd = info->lpDirectory;

		dwAttr = FILE_ATTRIBUTE_DIRECTORY;
		fa = GetFileAttributes(wd);
		if (!(fa & dwAttr))
			wd = info->lpDirectory;

		exec_program(wd, NULL, NULL, NORMALMODE,
			"git", "gui", NULL);
		
		return S_OK;
	}

	return E_INVALIDARG;
}

static STDMETHODIMP get_command_string(void *p, UINT id,
				       UINT flags, UINT *reserved,
				       LPSTR name, UINT size)
{

	struct git_menu *this_menu = p;
	struct git_data *this_ = this_menu->git_data;

	if (id != 1)
		return E_INVALIDARG;


	if (flags & GCS_HELPTEXT) {
		LPCTSTR text = _T("Launch the GIT Gui in the local or chosen directory.");
		size_t len = strlen(text) + 1;
		LPWSTR tw = malloc(len * sizeof(wchar_t));
		/* need to convert terminating NULL as well */
		mbstowcs(tw, text, len);
		/* use Win32 lstrcpyn to [automatically] avoid buffer overflow */
		if (flags & GCS_UNICODE)			
			lstrcpynW((LPWSTR)name, tw, size);
		else
			lstrcpynA(name, text, size);
		free(tw);
		return S_OK;
	}


	return E_INVALIDARG;
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
