#include "git-compat-util.h"
#include "systeminfo.h"
#include "debug.h"

static FILE *debug_git_fp = NULL;

void debug_git(char * format, ...)
{
	if (!debug_git_fp) {
#ifdef _WIN32
		WCHAR path[MAX_PATH];
		GetTempPathW(MAX_PATH, path);
		wcsncat(path, L"git_shell_ext_debug.txt", MAX_PATH);
		debug_git_fp = _wfopen(path, L"a+");
#else
		debug_git_fp = fopen("/tmp/git-cheetah-plugin.log", "a+");
#endif
	}

	/* Check again in case the above debug_git_set_file failed. */
	if (debug_git_fp)
	{
		va_list params;
		char buffer[1024];
		int length = 0;
      
		va_start(params, format);
		length = vsnprintf(buffer, sizeof(buffer), format, params);
		va_end(params);
		fwrite(buffer, sizeof(char), length, debug_git_fp);
		fputc('\n', debug_git_fp);
		fflush(debug_git_fp);
	}
}

void debug_git_mbox(char *format, ...)
{
	va_list params;
	char buffer[1024];

	va_start(params, format);
	vsnprintf(buffer, sizeof(buffer), format, params);
	va_end(params);
	message_box(buffer);
}
