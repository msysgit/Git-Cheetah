#include <shlobj.h>
#include <stdarg.h>
#include <tchar.h>
#include "menu.h"
#include "ext.h"
#include "debug.h"
#include "systeminfo.h"

/*
 * These are the functions for handling the context menu.
 */

static STDMETHODIMP query_context_menu(void *p, HMENU menu,
				       UINT index, UINT first_command,
				       UINT last_command, UINT flags)
{
	struct git_menu *this_menu = p;
	struct git_data *this_ = this_menu->git_data;

	if (flags & CMF_DEFAULTONLY)
		return MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_NULL, 0);

	InsertMenu(menu, index, MF_SEPARATOR | MF_BYPOSITION,
			first_command, "");
	InsertMenu(menu, index+1, MF_STRING | MF_BYPOSITION,
		   first_command+1, _T("&Git Gui"));
	
	return MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_NULL, 2);
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
static char * convert_directory_format(const char * path)
{
	int i;
	int size_incr = 0;

	/* Figure out how much extra space we need to escape spaces */
	for (i = 0; i < MAX_PATH && path[i] != '\0'; ++i)
		if (path[i] == ' ')
			size_incr++;

	char * converted = (char *)calloc(size_incr + i + 1, sizeof(char));
	char * dst = converted;

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
		STARTUPINFO si = { sizeof(si) };
		PROCESS_INFORMATION pi;
		
		TCHAR * msysPath = msys_path();

		if (msysPath)
		{
			TCHAR command[1024];
			
			wsprintf(command, TEXT("%s\\bin\\wish.exe \"%s/bin/git-gui\""),
				 msysPath, msysPath);
			
			
			const char *wd = this_->name;
			if (wd == NULL || strlen(wd) == 0)
				wd = info->lpDirectory;

			DWORD dwAttr = FILE_ATTRIBUTE_DIRECTORY;
			DWORD fa = GetFileAttributes(wd);
			if (! (fa & dwAttr))
				wd = info->lpDirectory;

			debug_git("Trying to spawn '%s' in working directory '%s'\n", command, wd);
			if (CreateProcess(
				    NULL,
				    command,
				    NULL,
				    NULL,
				    FALSE,
				    0, NULL, wd, &si, &pi))
			{
				CloseHandle(pi.hProcess);
				CloseHandle(pi.hThread);
			}
			else
			{
				debug_git("[ERROR] %s/%s:%d Could not create git gui process (%d) Command: %s",
					  __FILE__, __FUNCTION__, __LINE__,
					  GetLastError(), command);
			}
		}
		else
		{
			debug_git("[ERROR] %s/%s:%d Could not find msysPath",
				  __FILE__, __FUNCTION__, __LINE__);
		}
		
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
		LPWSTR tw = malloc((strlen(text)+1)*sizeof(wchar_t));
		mbstowcs(tw, text, strlen(text));
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
