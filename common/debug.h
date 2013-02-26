#ifndef DEBUG_H
#define DEBUG_H

typedef void reporter(char *format, ...);
void _debug_git(char * format, ...);
void _debug_git_mbox(char *format, ...);

#ifdef DEBUG
#	define debug_git _debug_git
#	define debug_git_mbox _debug_git_mbox
#else
#	define debug_git(...)
#	define debug_git_mbox
#endif

#endif /* DEBUG_H */
