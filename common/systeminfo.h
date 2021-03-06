#ifndef SYSTEMINFO_H
#define SYSTEMINFO_H


/* git_path returns additions to the systems path variable and is used
 * by exec_program for the environment. If an implementation does not
 * want to add anything to the path it should return an empty string. An
 * error is indicated by returning NULL.
 */
const char *git_path();

/* opens a message box with ok button on this platform */
void message_box(const char *string);

/* returns true if path is a directory */
int is_directory(const char *path);

pid_t fork_process(const char *cmd, const char **args, const char *wd);

/* returns 1 on success 0 on timeout and -1 on failure to get exit code
 */
int wait_for_process(pid_t pid, int max_time, int *errcode);

#ifndef _WIN32
#define close_process(pid) (void)(pid)
#else
void close_process(pid_t pid);
#endif

#endif /* SYSTEMINFO_H */
