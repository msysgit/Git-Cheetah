#ifndef DEBUG_H
#define DEBUG_H

#ifdef _WIN32
#define DEFAULT_DEBUG_GIT_FILE "C:/git_shell_ext_debug.txt"
#endif

/* default in case no special path is needed */
#ifndef DEFAULT_DEBUG_GIT_FILE
#define DEFAULT_DEBUG_GIT_FILE "/tmp/git-cheetah-plugin.log"
#endif

void debug_git_set_file(const char * filename);

typedef void reporter(char *format, ...);
void debug_git(char * format, ...);
void debug_git_mbox(char *format, ...);

#endif /* DEBUG_H */
