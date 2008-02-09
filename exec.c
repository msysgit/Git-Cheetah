#include <windows.h>
#include <stdio.h>
#include <process.h>
#include "debug.h"
#include "systeminfo.h"
#include "exec.h"

#define MAX_PROCESSING_TIME (60 * 1000)

char *env_for_git()
{
	static char *environment;

	/*
	 * if we can't find path to msys in the registry, return NULL and
	 * the CreateProcess will copy the environment for us
	 */
	if (!environment && msys_path()) {
		char *old = GetEnvironmentStrings();
		size_t space = 0, path_index = -1, name_len = 0, len2;

		while (old[space]) {
			/* if it's PATH variable (could be Path= too!) */
			if (!strnicmp(old + space, "PATH=", 5)) {
				path_index = space;
				name_len = 5;
			}

			while (old[space])
				space++;
			space++; /* skip var-terminating NULL */
		}

		if (path_index == -1)
			path_index = space;

		environment = malloc(space +
				2 * strlen(msys_path()) + 32);

		/* copy the block up to the equal sign of PATH var */
		memcpy(environment, old, path_index);
		/* insert new segments of the PATH */
		len2 = sprintf(environment + path_index,
			"PATH=%s\\bin;%s\\mingw\\bin%s",
			msys_path(), msys_path(), name_len ? ";" : "");
		/* append original value of PATH and variables after it */
		memcpy(environment + path_index + len2,
			old + path_index + name_len,
			space + 1 - path_index - name_len);

		FreeEnvironmentStrings(old);
	}

	return environment;
}

int exec_git(char *command, const char *wd, int mode)
{
	STARTUPINFO si = { sizeof(si) };
	PROCESS_INFORMATION pi;
	DWORD status = -1;
	char cmd[1024];

	if (!msys_path()) {
		debug_git("[ERROR] Could not find msysPath");
		return -1;
	}

	sprintf(cmd, "\"%s\\bin\\git.exe\" %s", msys_path(), command);

	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_HIDE;

	debug_git("Trying to spawn 'git %s' in working directory '%s'\n",
		command, wd);
	if (!CreateProcess(NULL, cmd, NULL, NULL, FALSE, 0,
			env_for_git(), wd, &si, &pi)) {
		debug_git("[ERROR] Could not create process (%d); "
			"wd: %s; cmd: %s",
			GetLastError(), wd, command);

		return -1;
	}

	if (P_WAIT == mode) {
		if (WAIT_OBJECT_0 == WaitForSingleObject(pi.hProcess,
				MAX_PROCESSING_TIME)) {

			if (!GetExitCodeProcess(pi.hProcess, &status))
				debug_git("[ERROR] GetExitCode failed (%d); "
					"wd: %s; cmd: %s",
					GetLastError(), wd, command);

		} else
			debug_git("[ERROR] process timed out; "
				"wd: %s; cmd: %s",
				wd, command);
	}

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	return status;
}

