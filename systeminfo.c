#include <windows.h>
#include "systeminfo.h"

static TCHAR msysPath[MAX_PATH];

static int get_msys_path_from_registry(HKEY root)
{
	HKEY key;
	DWORD path_len = sizeof(msysPath);

	LONG result = RegOpenKeyEx(root, GIT_CHEETAH_REG_PATH,
		0, KEY_QUERY_VALUE, &key);
	if (ERROR_SUCCESS != result)
		return 0;

	result = RegQueryValueEx(key,
		GIT_CHEETAH_REG_PATHTOMSYS,
		NULL, NULL, (LPBYTE)msysPath, &path_len);

	RegCloseKey(key);

	return ERROR_SUCCESS == result;
}

TCHAR *msys_path(void)
{
	static int found_path = 0;

	/* try to find user-specific settings first */
	if (!found_path)
		found_path = get_msys_path_from_registry(HKEY_CURRENT_USER);

	/* if not found in user settings, try machine-wide */
	if (!found_path)
		found_path = get_msys_path_from_registry(HKEY_LOCAL_MACHINE);

	if (found_path)
		return msysPath;

	return NULL;
}
