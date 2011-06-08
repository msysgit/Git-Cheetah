#ifndef DEBUG_H
#define DEBUG_H

typedef void reporter(char *format, ...);
void debug_git(char * format, ...);
void debug_git_mbox(char *format, ...);

#endif /* DEBUG_H */
