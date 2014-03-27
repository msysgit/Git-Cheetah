#pragma once

#ifdef _WIN32

/* Windows-specific implementation of exec_program_v() */

#include "cache.h"
#include "debug.h"
#include "exec.h"
#include "git-compat-util.h"
#include "strbuf.h"
#include "systeminfo.h"

struct output_rec {
	HANDLE out;
	HANDLE err;
};

struct handles_rec {
	HANDLE in;
	HANDLE out;
	HANDLE err;
};

struct path_rec {
	char *envpath;
	char *envpath_old;
};

struct cmd_rec {
	char *cmd;
	char *args;
};

#define MAX_PROCESSING_TIME	(60 * 1000)

static char **path_split(const char *envpath)
{
	char *orig, *p, **path;
	int i, n = 0;

	orig = xstrdup(envpath);
	p = orig;

	while (p) {
		char *dir = p;
		p = strchr(p, ';');

		if (p) {
			*p++ = '\0';
		}

		if (*dir) {	/* not earlier, catches series of ; */
			++n;
		}
	}

	if (!n) {
		return NULL;
	}

	path = (char **)xmalloc((n + 1) * sizeof(char *));
	p = orig;
	i = 0;

	do {

		if (*p) {
			path[i++] = xstrdup(p);
		}
		p = p + strlen(p) + 1;

	} while (i < n);

	path[i] = NULL;

	return path;
}

static char *path_lookup_prog(const char *dir, const char *cmd)
{
	char path[MAX_PATH];
	int len = strlen(cmd);
	int isexe = len >= 4 && 0 == strcasecmp(cmd + len - 4, ".exe");

	if (isexe) {
		snprintf(path, sizeof(path), "%s/%s", dir, cmd);
	} else {
		snprintf(path, sizeof(path), "%s/%s.exe", dir, cmd);
	}

	if (0 == access(path, F_OK)) {
		return xstrdup(path);
	}

	return NULL;
}

/*
 * Determines the absolute path of cmd using the the split path in path.
 * If cmd contains a slash or backslash, no lookup is performed.
 */
static char *path_lookup(const char *cmd, char **path)
{
	char *prog = NULL;

	if (strchr(cmd, '/') || strchr(cmd, '\\')) {
		return xstrdup(cmd);
	}

	while (!prog && *path) {
		prog = path_lookup_prog(*path++, cmd);
	}

	return prog;
}

static const char *escape_arg(const char *arg)
{
	/* count chars to quote */
	int len = 0, n = 0;
	int force_quotes = 0;
	char *q, *d;
	const char *p = arg;
	if (!*p) force_quotes = 1;
	while (*p) {
		if (isspace(*p) || *p == '*' || *p == '?' || *p == '{')
			force_quotes = 1;
		else if (*p == '"')
			n++;
		else if (*p == '\\') {
			int count = 0;
			while (*p == '\\') {
				count++;
				p++;
				len++;
			}
			if (*p == '"')
				n += count*2 + 1;
			continue;
		}
		len++;
		p++;
	}
	if (!force_quotes && n == 0) {
		return arg;
	}

	/* insert \ where necessary */
	d = q = (char *)xmalloc(len+n+3);
	*d++ = '"';
	while (*arg) {
		if (*arg == '"')
			*d++ = '\\';
		else if (*arg == '\\') {
			int count = 0;
			while (*arg == '\\') {
				count++;
				*d++ = *arg++;
			}
			if (*arg == '"') {
				while (count-- > 0)
					*d++ = '\\';
				*d++ = '\\';
			}
		}
		*d++ = *arg++;
	}
	*d++ = '"';
	*d++ = 0;
	return q;
}

static void cmd_rec_init(const char **argv, const char *envpath,
	struct cmd_rec *rec)
{
	struct strbuf args;
	char *escaped;
	char **path = path_split(envpath);
	strbuf_init(&args, 0);

	rec->cmd = path_lookup(*argv, path);

	if (!rec->cmd) {
		return;
	}

	for (; *argv; argv++) {
		escaped = (char *)escape_arg(*argv);
		if (args.len) {
			strbuf_addch(&args, ' ');
		}
		strbuf_addstr(&args, escaped);
		if (escaped != *argv) {
			free(escaped);
		}
	}

	rec->args = strbuf_detach(&args, NULL);
}

static void cmd_rec_final(struct cmd_rec *rec)
{
	if (rec->cmd) {
		free(rec->cmd);
	}

	if (rec->args) {
		free(rec->args);
	}
}

static BOOL create_output_handles(PHANDLE hRead, PHANDLE hWrite, HANDLE hProc)
{
	HANDLE hWriteTmp;

	/* create pipe with no inheritance */
	if (!CreatePipe(hRead, &hWriteTmp, NULL, 0)) {
		return FALSE;
	}

	/* dup write end with inheritance to hWrite and close hWriteTmp */
	if (!DuplicateHandle(hProc, hWriteTmp, hProc, hWrite, 0, TRUE,
		DUPLICATE_SAME_ACCESS | DUPLICATE_CLOSE_SOURCE)) {
		CloseHandle(hWriteTmp);
		return FALSE;
	}

	return TRUE;
}

static BOOL create_handles(struct handles_rec *hStd, struct output_rec *hOutput)
{
	HANDLE hProc = GetCurrentProcess();
	hStd->in = GetStdHandle(STD_INPUT_HANDLE);

	/* create stdOut */
	if (!create_output_handles(&hOutput->out, &hStd->out, hProc)) {
		return FALSE;
	}

	/* create stdErr */
	if (!create_output_handles(&hOutput->err, &hStd->err, hProc)) {
		return FALSE;
	}

	return TRUE;
}

static BOOL has_console(void)
{
	HANDLE con = CreateFile("CONOUT$", GENERIC_WRITE, FILE_SHARE_WRITE,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (con == INVALID_HANDLE_VALUE) {
		return FALSE;
	} else {
		CloseHandle(con);
		return TRUE;
	}
}

static void path_rec_init(const char *gitpath, struct path_rec *rec)
{
	struct strbuf path = STRBUF_INIT;

	if (gitpath) {
		strbuf_addstr(&path, gitpath);
	}

	if ((rec->envpath_old = getenv("PATH"))) {
		rec->envpath_old = xstrdup(rec->envpath_old);
		strbuf_addch(&path, ';');
		strbuf_addstr(&path, rec->envpath_old);
	}

	if (path.len) {
		setenv("PATH", path.buf, 1);
		rec->envpath = strbuf_detach(&path, NULL);
	}
}

static void path_rec_final(struct path_rec *rec)
{
	if (rec->envpath_old) {
		setenv("PATH", rec->envpath_old, 1);
		free(rec->envpath_old);
	}

	if (rec->envpath) {
		free(rec->envpath);
	}
}

static void path_split_free(char **path)
{
	char **p;

	if (!path) {
		return;
	}

	p = path;
	while (*p) {
		free(*p++);
	}

	free(path);
}

static HANDLE process_init(struct cmd_rec cmdInfo, const char *dir,
	struct handles_rec hStd)
{
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	unsigned flags;
	BOOL success;

	flags = has_console() ? 0 : CREATE_NO_WINDOW;

	memset(&pi, 0, sizeof(pi));
	memset(&si, 0, sizeof(si));
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESTDHANDLES;
	si.hStdInput = hStd.in;
	si.hStdOutput = hStd.out;
	si.hStdError = hStd.err;

	success = CreateProcess(cmdInfo.cmd, cmdInfo.args, NULL, NULL,
		TRUE, flags, NULL, dir, &si, &pi);

	if (!success) {
		errno = ENOENT;
	}

	/* close our end of the output handles */
	CloseHandle(hStd.out);
	CloseHandle(hStd.err);

	CloseHandle(pi.hThread);

	return success ? pi.hProcess : INVALID_HANDLE_VALUE;
}

static int process_final(HANDLE hProc, int wait, struct strbuf *output,
	struct strbuf *error, struct output_rec hOutput)
{
	int res, status = 0;

	if (wait) {

		res = wait_for_process((pid_t)hProc, MAX_PROCESSING_TIME, &status);

		if (res) {

			if (res < 0) {
				status = -1;
			}

			if (output) {
				int fd = _open_osfhandle((intptr_t)hOutput.out, _O_RDONLY);
				strbuf_read(output, fd, 0);
			}

			if (error) {
				int fd = _open_osfhandle((intptr_t)hOutput.err, _O_RDONLY);
				strbuf_read(error, fd, 0);
			}

		} else {
			status = -ERR_RUN_COMMAND_WAITPID_NOEXIT;
		}
	}

	CloseHandle(hProc);
	CloseHandle(hOutput.out);
	CloseHandle(hOutput.err);

	return status;
}

int exec_program_v(const char *working_directory, struct strbuf *output,
	struct strbuf *error, int flags, const char **argv)
{
	struct handles_rec hStd;
	struct output_rec hOutput;
	struct path_rec pathInfo = {NULL, NULL};
	struct cmd_rec cmdInfo = {NULL, NULL};
	const char *gitpath;
	HANDLE hProc;
	int wait, status = 0;

	if (!(gitpath = git_path())) {
		return -1;
	}

	if (!create_handles(&hStd, &hOutput)) {
		return -1;
	}

	path_rec_init(gitpath, &pathInfo);
	cmd_rec_init(argv, pathInfo.envpath, &cmdInfo);

	if (cmdInfo.cmd) {
		hProc = process_init(cmdInfo, working_directory, hStd);

		if (INVALID_HANDLE_VALUE != hProc) {
			wait = WAITMODE & flags;
			status = process_final(hProc, wait, output, error, hOutput);
		}
	}

	cmd_rec_final(&cmdInfo);
	path_rec_final(&pathInfo);

	return status;
}
#endif
