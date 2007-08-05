#include <windows.h>
#include "systeminfo.h"

static TCHAR msysPath[MAX_PATH];

TCHAR * msys_path(void)
{
	static int found_path = 0;
	HKEY hKey;
	LONG lRet;

	/* Only bother to get it once. */
	if (found_path)
		return msysPath;
	
	lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
			    TEXT(GIT_CHEETAH_REG_PATH),
			    0, KEY_QUERY_VALUE, &hKey);
	
	if (lRet == ERROR_SUCCESS)
	{
		DWORD msysPathLen = MAX_PATH * sizeof(TCHAR);
		
		lRet = RegQueryValueEx(hKey,
				       TEXT(GIT_CHEETAH_REG_PATHTOMSYS),
				       NULL, NULL,
				       (LPBYTE)msysPath,
				       &msysPathLen);
		RegCloseKey(hKey);

		if (lRet == ERROR_SUCCESS)
		{
			found_path = 1;
			return msysPath;
		}
	}

	return NULL;
}
