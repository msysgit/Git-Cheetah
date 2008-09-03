#ifndef CACHE_H
#define CACHE_H

#include "git-compat-util.h"
#include "strbuf.h"

enum date_mode {
	DATE_NORMAL = 0,
	DATE_RELATIVE,
	DATE_SHORT,
	DATE_LOCAL,
	DATE_ISO8601,
	DATE_RFC2822
};

#define alloc_nr(x) (((x)+16)*3/2)

/*
 * Realloc the buffer pointed at by variable 'x' so that it can hold
 * at least 'nr' entries; the number of entries currently allocated
 * is 'alloc', using the standard growing factor alloc_nr() macro.
 *
 * DO NOT USE any expression with side-effect for 'x' or 'alloc'.
 */
#define ALLOC_GROW(x, nr, alloc) \
	do { \
		if ((nr) > alloc) { \
			if (alloc_nr(alloc) < (nr)) \
				alloc = (nr); \
			else \
				alloc = alloc_nr(alloc); \
			x = xrealloc((x), alloc * sizeof(*(x))); \
		} \
	} while(0)

struct pack_window {
	struct pack_window *next;
	unsigned char *base;
	off_t offset;
	size_t len;
	unsigned int last_used;
	unsigned int inuse_cnt;
};

extern struct packed_git {
	struct packed_git *next;
	struct pack_window *windows;
	off_t pack_size;
	const void *index_data;
	size_t index_size;
	uint32_t num_objects;
	uint32_t num_bad_objects;
	unsigned char *bad_object_sha1;
	int index_version;
	time_t mtime;
	int pack_fd;
	int pack_local;
	unsigned char sha1[20];
	/* something like ".git/objects/pack/xxxxx.pack" */
	char pack_name[FLEX_ARRAY]; /* more */
} *packed_git;

#endif /* CACHE_H */
