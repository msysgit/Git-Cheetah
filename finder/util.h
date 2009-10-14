#ifndef FINDER_UTIL_H
#define FINDER_UTIL_H

/* on Mac OS 10.4 there is a struct strbuf in Carbon */
#define strbuf mac_strbuf
#include <Carbon/Carbon.h>
#undef strbuf
#include "../common/strbuf.h"

/* implements platform dependent utility functions for
 * handling platform dependent types */
int selection_to_path(char *path, int len, const AEDesc *selection /*,
		int *index */);

AEDesc *get_file_from_selection(const AEDesc *selection, AEDesc *file);
AEDesc *get_file_from_selection_list(const AEDesc *selection, AEDesc *file
		/* , int *index */);
int exec_program(const char *working_directory,
	struct strbuf *output, struct strbuf *error_output,
	int flags, ...);

#endif /* FINDER_UTIL_H */
