#include <stdarg.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include "git_shell_ext_debug.h"

static char debug_git_file[MAX_PATH];
static FILE * debug_git_fd = NULL;

void debug_git_set_file(const char * filename)
{
        if (debug_git_fd)
	{
		fclose(debug_git_fd);
		debug_git_fd = NULL;
	}
   
	strncpy(debug_git_file, filename, MAX_PATH);
	debug_git_file[MAX_PATH-1] = '\0';

	debug_git_fd = fopen(debug_git_file, "a+");
}

void debug_git(char * format, ...)
{
	if (!debug_git_fd)
	{
		debug_git_set_file(DEFAULT_DEBUG_GIT_FILE);
	}

	/* Check again in case the above debug_git_set_file failed. */
	if (debug_git_fd)
	{
		va_list params;
		char buffer[1024];
		int length = 0;
      
		va_start(params, format);
		length = vsnprintf(buffer, sizeof(buffer), format, params);
		va_end(params);
		fwrite(buffer, sizeof(char), length, debug_git_fd);
		fflush(debug_git_fd);
	}
}
