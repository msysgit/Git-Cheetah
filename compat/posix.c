/* posix.c
 *
 * Implementation of git-cheetah compatibility functions for POSIX
 *
 * Copyright (C) Heiko Voigt, 2009
 *
 */
#include "../common/git-compat-util.h"
#include <dirent.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include "../common/strbuf.h"
#include "../common/exec.h"
#include "../common/strbuf.h"
#include "../common/debug.h"
#include "../common/systeminfo.h"

#define MAX_ARGS 32

int is_directory(const char *path)
{
	struct stat st;
	return !lstat(path, &st) && S_ISDIR(st.st_mode);
}

pid_t fork_process(const char *cmd, const char **args, const char *wd)
{
	pid_t pid;
	char *line;
	char *path_variable;
	int i, line_max;

	pid = fork();
	if (pid)
		return pid;

	if (chdir(wd) < 0)
		debug_git("chdir failed: %s", strerror(errno));

	path_variable = getenv("PATH");
	line_max = strlen(path_variable)+MAX_PATH+2;
	line = xmalloc(line_max*sizeof(char));
	snprintf(line, line_max, "%s%s%s", path_variable,
			*(git_path()) ? ":" : "", git_path());
	setenv("PATH", line, 1);

	snprintf(line, line_max, "%s", cmd);
	for (i=1; args[i] && i<MAX_ARGS; i++) {
		strncat(line, " ", 1024);
		strncat(line, args[i], 1024);
	}
	line[line_max-1] = '\0';
	debug_git("starting child, wd: '%s' call: '%s'", wd, line);
	free(line);
	/* We are already forked here and we usually never return from
	 * the next call so it does not matter wether we cast to (char**)
	 * to suppress warnings because we can not modify the callers
	 * values anyway.
	 */
	execvp(cmd,(char **)args);

	/* here this code is done, if not something went wrong */
	fprintf(stderr, "execv failed: %s, Error: %s\n", cmd, strerror(errno));
	exit(-ERR_RUN_COMMAND_FORK);
}

int wait_for_process(pid_t pid, int max_time, int *errcode)
{
	int stat_loc;
	if (waitpid(pid, &stat_loc, 0) < 0) {
		debug_git("waitpid failed (%i): %s",errno, strerror(errno));
		*errcode = -ERR_RUN_COMMAND_WAITPID_NOEXIT;
		return -1;
	}

	if (WIFEXITED(stat_loc)) {
		*errcode = WEXITSTATUS(stat_loc);
		debug_git("Exit status: 0x%x", *errcode);
		return 1;
	} else {
		char errmsg[1024];
		*errcode = -1;
		if (WIFSIGNALED(stat_loc) && WCOREDUMP(stat_loc))
			sprintf(errmsg,"coredump");
		else if (WIFSIGNALED(stat_loc))
			sprintf(errmsg,"terminated due to signal %i", WTERMSIG(stat_loc));
		else if (WIFSTOPPED(stat_loc))
			sprintf(errmsg,"stopped due to signal %i", WSTOPSIG(stat_loc));
		debug_git("[ERROR] child process failed: %s", errmsg);
		return -1;
	}
	return 0;
}
