#include "cache.h"

#include "debug.h"
#include "systeminfo.h"
#include "exec.h"

#define MAX_PROCESSING_TIME	(60 * 1000)
#define MAX_ARGS		32

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
	if (!environment && msys_path()) {
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
					2 * strlen(msys_path()) + 32);
				len = sprintf(environment[i],
					"PATH=%s\\bin;%s\\mingw\\bin%s%s",
					msys_path(), msys_path(),
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

/* copy from run-command.c */
static inline void close_pair(int fd[2])
{
	close(fd[0]);
	close(fd[1]);
}

int exec_program(const char *working_directory,
	struct strbuf *output, struct strbuf *error_output,
	int flags, ...)
{
	int fdout[2], fderr[2];
	int s0 = -1, s1 = -1, s2 = -1;	/* backups of stdin, stdout, stderr */

	va_list params;
	const char *argv[MAX_ARGS];
	char *arg;
	int argc = 0;

	pid_t pid;
	DWORD status = 0;

	reporter *debug = QUIETMODE & flags ? debug_git : debug_git_mbox;

	if (!msys_path()) {
		debug("[ERROR] Could not find msysPath");
		return -1;
	}

	if (output) {
		if (pipe(fdout) < 0) {
			return -ERR_RUN_COMMAND_PIPE;
		}
		s1 = dup(1);
		dup2(fdout[1], 1);

		flags |= WAITMODE;
	}

	if (error_output) {
		if (pipe(fderr) < 0) {
			if (output)
				close_pair(fdout);
			return -ERR_RUN_COMMAND_PIPE;
		}
		s2 = dup(2);
		dup2(fderr[1], 2);

		flags |= WAITMODE;
	}

	va_start(params, flags);
	do {
		arg = va_arg(params, char*);
		argv[argc++] = arg;
	} while (argc < MAX_ARGS && arg);
	va_end(params);

	pid = mingw_spawnvpe_cwd(argv[0], argv, env_for_git(),
		working_directory);

	if (s1 >= 0)
		dup2(s1, 1), close(s1);
	if (s2 >= 0)
		dup2(s2, 2), close(s2);

	if (pid < 0) {
		if (output)
			close_pair(fdout);
		if (error_output)
			close_pair(fderr);
		return -ERR_RUN_COMMAND_FORK;
	}

	if (output)
		close(fdout[1]);
	if (error_output)
		close(fderr[1]);

	if (WAITMODE & flags) {
		if (WAIT_OBJECT_0 == WaitForSingleObject((HANDLE)pid,
			MAX_PROCESSING_TIME)) {
			if (GetExitCodeProcess((HANDLE)pid, &status))
				debug_git("Exit code: %d", status);
			else {
				/* play safe, and return total failure */
				status = -1;
				debug_git("[ERROR] GetExitCode failed (%d); "
					"wd: %s; cmd: %s",
					GetLastError(),
					working_directory,
					argv[0]);
			}

			if (output) {
				strbuf_read(output, fdout[0], 0);
				debug_git("STDOUT:\r\n%s\r\n*** end of STDOUT ***\r\n", output->buf);
			}

			if (error_output) {
				strbuf_read(error_output, fderr[0], 0);
				debug_git("STDERR:\r\n%s\r\n*** end of STDERR ***\r\n", error_output->buf);
			}
		} else {
			status = -ERR_RUN_COMMAND_WAITPID_NOEXIT;
			debug_git("[ERROR] process timed out; "
				"wd: %s; cmd: %s",
				working_directory, argv[0]);
		}
	}

	if (output)
		close(fdout[0]);
	if (error_output)
		close(fderr[0]);

	return status;
}
