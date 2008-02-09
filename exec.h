
/*
 * Modifies a copy of the environment to include Git in PATH
 */
char *env_for_git();

/*
 * Executes a console application hidden as in "git gui" to hide git
 */
void exec_gui(char *cmd, const char *wd);

