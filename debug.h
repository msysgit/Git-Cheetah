#ifndef DEBUG_H
#define DEBUG_H

#define DEFAULT_DEBUG_GIT_FILE "C:/git_shell_ext_debug.txt"

void debug_git_set_file(const char * filename);
void debug_git(char * format, ...);
void debug_git_mbox(char *format, ...);

#endif /* DEBUG_H */
