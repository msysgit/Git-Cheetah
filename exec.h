
/*
 * Modifies a copy of the environment to include Git in PATH
 */
char *env_for_git();

/*
 * Native, simplified implementation of spawn with a working directory
 *
 * Executes git.exe
 * Supports only P_WAIT and P_NOWAIT modes.
 */
int exec_git(char *cmd, const char *wd, int mode);

