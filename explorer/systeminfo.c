#include "../common/git-compat-util.h"
#include <windows.h>
#include "../common/systeminfo.h"
#include "dll.h"

static char gitPath[MAX_PATH] = "";
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

static TCHAR *msys_path(void)
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

static inline int test_msys_path(const char *postfix, char *path)
{
	int n;
	char stat_path[MAX_PATH];
	struct stat stat_info;

	n = snprintf(stat_path, MAX_PATH, "%s\\%s\\git",
			msys_path(), postfix);
	if (n >= MAX_PATH)
		goto test_git_path_failed;

	n = snprintf(path, MAX_PATH, "%s\\%s", msys_path(), postfix);
	if (n >= MAX_PATH)
		goto test_git_path_failed;

	if (lstat(path, &stat_info) == 0) {
		return 1;
	}

test_git_path_failed:
	*path = '\0';
	return 0;
}

const char *git_path()
{
	char path[MAX_PATH];

	if (!msys_path())
		return NULL;

	if (*gitPath)
		return gitPath;

	if (test_msys_path("bin", path)) {
		memcpy(gitPath, path, MAX_PATH);
		return gitPath;
	}

	if (test_msys_path("mingw\\bin", path)) {
		memcpy(gitPath, path, MAX_PATH);
		return gitPath;
	}

	return NULL;
}
