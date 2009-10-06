
#include "cache.h"

static unsigned int pack_open_windows;
static size_t pack_mapped;
struct packed_git *packed_git;

static void scan_windows(struct packed_git *p,
	struct packed_git **lru_p,
	struct pack_window **lru_w,
	struct pack_window **lru_l)
{
	struct pack_window *w, *w_l;

	for (w_l = NULL, w = p->windows; w; w = w->next) {
		if (!w->inuse_cnt) {
			if (!*lru_w || w->last_used < (*lru_w)->last_used) {
				*lru_p = p;
				*lru_w = w;
				*lru_l = w_l;
			}
		}
		w_l = w;
	}
}

static int unuse_one_window(struct packed_git *current, int keep_fd)
{
	struct packed_git *p, *lru_p = NULL;
	struct pack_window *lru_w = NULL, *lru_l = NULL;

	if (current)
		scan_windows(current, &lru_p, &lru_w, &lru_l);
	for (p = packed_git; p; p = p->next)
		scan_windows(p, &lru_p, &lru_w, &lru_l);
	if (lru_p) {
		munmap(lru_w->base, lru_w->len);
		pack_mapped -= lru_w->len;
		if (lru_l)
			lru_l->next = lru_w->next;
		else {
			lru_p->windows = lru_w->next;
			if (!lru_p->windows && lru_p->pack_fd != keep_fd) {
				close(lru_p->pack_fd);
				lru_p->pack_fd = -1;
			}
		}
		free(lru_w);
		pack_open_windows--;
		return 1;
	}
	return 0;
}

void release_pack_memory(size_t need, int fd)
{
	size_t cur = pack_mapped;
	while (need >= (cur - pack_mapped) && unuse_one_window(NULL, fd))
		; /* nothing */
}
