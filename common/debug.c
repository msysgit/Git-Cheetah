#include "git-compat-util.h"
#include "systeminfo.h"
#include "debug.h"

static FILE *debug_git_fp = NULL;

#ifdef _WIN32
static void reset_inherit_flag(FILE *file)
{
	HANDLE handle;

	if(!file)
		return;

	handle = (HANDLE)_get_osfhandle(_fileno(file));
	SetHandleInformation(handle, HANDLE_FLAG_INHERIT, 0);
}
#endif

void _debug_git(char * format, ...)
{
	if (!debug_git_fp) {
#ifdef _WIN32
		WCHAR path[MAX_PATH];
		GetTempPathW(MAX_PATH, path);
		wcsncat(path, L"git_shell_ext_debug.txt", MAX_PATH);
		debug_git_fp = _wfopen(path, L"a+");
		reset_inherit_flag(debug_git_fp);
#else
		debug_git_fp = fopen("/tmp/git-cheetah-plugin.log", "a+");
#endif
	}

	/* Check again in case the above debug_git_set_file failed. */
	if (debug_git_fp)
	{
		va_list params;
		char *buffer;
		int length = 0;
      
		va_start(params, format);
		length = vsnprintf(NULL, 0, format, params);
		if (length < 0)
			return;

		buffer = xmalloc(length + 1);
		vsnprintf(buffer, length, format, params);
		va_end(params);
		fwrite(buffer, sizeof(char), length, debug_git_fp);
		fputc('\n', debug_git_fp);
		fflush(debug_git_fp);
		free(buffer);
	}
}

void _debug_git_mbox(char *format, ...)
{
	va_list params;
	char buffer[1024];

	va_start(params, format);
	vsnprintf(buffer, sizeof(buffer), format, params);
	va_end(params);
	message_box(buffer);
}
