#include "../common/git-compat-util.h"
#include "../common/strbuf.h"
#include "../common/systeminfo.h"
#include "../common/debug.h"
#include <windows.h>
#include "dll.h"

static struct strbuf gitPath = STRBUF_INIT;
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

	if (gitPath.len)
		return gitPath.buf;

	if (test_msys_path("bin", path))
		strbuf_addstr(&gitPath, path);

	if (test_msys_path("mingw\\bin", path)) {
		if (gitPath.len)
			strbuf_addch(&gitPath, ';');
		strbuf_addstr(&gitPath, path);
	}

	return gitPath.len ? gitPath.buf : NULL;
}

void message_box(const char *string)
{
	MessageBox(0, string, "Hello", MB_OK|MB_ICONEXCLAMATION);
}

int is_directory(const char *path)
{
	return (FILE_ATTRIBUTE_DIRECTORY & GetFileAttributes(path));
}

pid_t fork_process(const char *cmd, const char **args, const char *wd)
{
	pid_t result;
	char *oldpath = NULL;
	const char *gitpath = git_path();

	/* if gitpath is set, temporarily set PATH=<gitpath>;%PATH% */
	if (gitpath) {
		struct strbuf path = STRBUF_INIT;
		strbuf_addstr(&path, gitpath);
		oldpath = getenv("PATH");
		if (oldpath) {
			oldpath = xstrdup(oldpath);
			strbuf_addch(&path, ';');
			strbuf_addstr(&path, oldpath);
		}
		setenv("PATH", path.buf, 1);
		strbuf_release(&path);
	}

	/* spawn child process */
	result = mingw_spawnvpe_cwd(cmd, args, NULL, wd);

	/* reset PATH to previous value */
	if (gitpath) {
		setenv("PATH", oldpath, 1);
		free(oldpath);
	}
	return result;
}

int wait_for_process(pid_t pid, int max_time, int *errcode)
{
	DWORD status;
	*errcode = 0;

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

void close_process(pid_t pid)
{
	CloseHandle((HANDLE)pid);
}
