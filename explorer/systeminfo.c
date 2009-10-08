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

void message_box(const char *string)
{
	MessageBox(0, string, "Hello", MB_OK|MB_ICONEXCLAMATION);
}

int is_path_directory(const char *path)
{
	return (FILE_ATTRIBUTE_DIRECTORY & GetFileAttributes(path));
}

/*
 * builds an array of environment variables,
 * as expected by mingw_spawnvpe
 */
static char **env_for_git()
{
	static char **environment;

	/*
	 * if we can't find path to msys in the registry, return NULL and
	 * the CreateProcess will copy the environment for us
	 */
	if (!environment && git_path()) {
		char *old = GetEnvironmentStrings();
		size_t space = 0, path_index = -1, name_len = 0;
		int total = 0, i;

		while (old[space]) {
			/* if it's PATH variable (could be Path= too!) */
			if (!strnicmp(old + space, "PATH=", 5)) {
				path_index = space;
				name_len = 5;
			}

			while (old[space])
				space++;
			space++; /* skip var-terminating NULL */

			total++;
		}

		if (path_index == -1)
			path_index = space;

		environment = malloc(sizeof(*environment) * (total + 1));
		space = 0;
		for (i = 0; i < total; i++) {
			if (path_index == space) {
				char *path = old + space + name_len;
				size_t len;
				environment[i] = malloc(strlen(path) + 1 +
					2 * strlen(git_path()) + 32);
				len = sprintf(environment[i],
					"PATH=%s%s%s", git_path(),
					*path ? ";" : "", path);
			} else
				environment[i] = strdup(old + space);

			while (old[space])
				space++;
			space++; /* skip var-terminating NULL */
		}

		/* mark the end of the array */
		environment[i] = 0;

		FreeEnvironmentStrings(old);
	}

	return environment;
}

pid_t fork_process(const char *cmd, const char **args, const char *wd)
{
	return mingw_spawnvpe_cwd(cmd, args, env_for_git(), wd);
}

int wait_for_process(pid_t pid, int max_time, int *errcode)
{
	*errcode = 0;
	DWORD status;

	if (WAIT_OBJECT_0 == WaitForSingleObject((HANDLE)pid,
				max_time)) {
		if (GetExitCodeProcess((HANDLE)pid, &status)) {
			*errcode = status;
			debug_git("Exit code: %d", status);
		}else {
			/* play safe, and return total failure */
			*errcode = GetLastError();
			return -1;
		}
		return 1;
	}
	return 0;
}
