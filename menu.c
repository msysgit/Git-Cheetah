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

	InsertMenu(menu, index, MF_STRING | MF_BYPOSITION,
		   first_command, _T("Git Gui"));

	return MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_NULL, 1);
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

	if (command == 0)
	{
		STARTUPINFO si = { sizeof(si) };
		PROCESS_INFORMATION pi;
		
		TCHAR * msysPath = msys_path();

		if (msysPath)
		{
			TCHAR command[1024];
			if (info->lpDirectory != NULL)
			{
				char * directory =
					convert_directory_format(info->lpDirectory);

				wsprintf(command, TEXT("%s\\bin\\sh.exe --login -c \"cd %s && /bin/git-gui\""),
					 msysPath, directory);
				free(directory);
			}
			else
			{
				wsprintf(command, TEXT("%s\\bin\\sh.exe --login /bin/git-gui"),
					 msysPath);
			}

			if (CreateProcess(
				    NULL,
				    command,
				    NULL,
				    NULL,
				    FALSE,
				    0, NULL, NULL, &si, &pi))
			{
				CloseHandle(pi.hProcess);
				CloseHandle(pi.hThread);
			}
			else
			{
				debug_git("[ERROR] %s/%s:%d Could not create git gui process (%d)",
					  __FILE__, __FUNCTION__, __LINE__,
					  GetLastError());
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

	if (id == 0)
		return E_INVALIDARG;

#if 0
	if (flags & GCS_HELPTEXT) {
		LPCTSTR text = _T("This is the simple shell extension's help");

		if (flags & GCS_UNICODE)
			lstrcpynW((LPWSTR)name, mbrtowc(text), size);
		else
			lstrcpynA(name, text, size);

		return S_OK;
	}
#endif

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
