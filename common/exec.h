
#define NORMALMODE (0)
#define HIDDENMODE (1 << 0) /* hide any windows, opened by execution */
#define WAITMODE   (1 << 1) /* wait for execution to complete */
#define DETACHMODE (1 << 2) /* do *not* wait for execution to complete */
#define QUIETMODE  (1 << 7) /* don't report any errors to user */

enum {
	ERR_RUN_COMMAND_FORK = 10000,
	ERR_RUN_COMMAND_EXEC,
	ERR_RUN_COMMAND_PIPE,
	ERR_RUN_COMMAND_WAITPID,
	ERR_RUN_COMMAND_WAITPID_WRONG_PID,
	ERR_RUN_COMMAND_WAITPID_SIGNAL,
	ERR_RUN_COMMAND_WAITPID_NOEXIT,
};

/*
 * Varargs interface to mingw_spawnvpe with NULL as end-of-format indicator.
 *
 * - supports specifying working directory;
 * - supports different modes of operation (see NORMALMODE & co. above).
 */
int exec_program(const char *working_directory,
	struct strbuf *output, struct strbuf *error_output,
	int flags, ...);

int exec_program_v(const char *working_directory,
	struct strbuf *output, struct strbuf *error_output,
	int flags, const char **argv);
