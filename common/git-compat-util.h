#ifndef GIT_COMPAT_UTIL_H
#define GIT_COMPAT_UTIL_H

#ifndef FLEX_ARRAY
/*
 * See if our compiler is known to support flexible array members.
 */
#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
# define FLEX_ARRAY /* empty */
#elif defined(__GNUC__)
# if (__GNUC__ >= 3)
#  define FLEX_ARRAY /* empty */
# else
#  define FLEX_ARRAY 0 /* older GNU extension */
# endif
#elif defined(_MSC_VER)
#define FLEX_ARRAY /* empty */
#endif

/*
 * Otherwise, default to safer but a bit wasteful traditional style
 */
#ifndef FLEX_ARRAY
# define FLEX_ARRAY 1
#endif
#endif

#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))

#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <stdarg.h>
#include <errno.h>
#include <limits.h>
#include <time.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/time.h>
#ifdef _WIN32
#include <io.h>
#include <sys/utime.h>
#include <process.h>
#endif /* _WIN32 */

#if defined(_MSC_VER)
/* MSVC defines mkdir here, not in io.h */
#include <direct.h>
#endif

#ifndef PATH_MAX
/*
 * with MSVC, if we define _POSIX_ we loose half of useful functions
 * (e.g. chmod, open, close), otherwise we don't have PATH_MAX
 * because of #ifdef in <io.h>.
 * So, when build under MSVC, don't define _POSIX_
 */
#define PATH_MAX            512
#endif

#ifdef _WIN32
#ifdef _SSIZE_T_DEFINED
#define _SSIZE_T_
#endif

/* from mingw/include/sys/types.h */
#ifndef _SSIZE_T_
#define _SSIZE_T_
typedef long _ssize_t;

#ifndef	_NO_OLDNAMES
typedef _ssize_t ssize_t;
#endif
#endif /* Not _SSIZE_T_ */
#ifndef _MODE_T_
#define	_MODE_T_
typedef unsigned short _mode_t;

#ifndef	_NO_OLDNAMES
typedef _mode_t	mode_t;
#endif
#endif	/* Not _MODE_T_ */


/* from mingw/include/io.h */
/* Some defines for _access nAccessMode (MS doesn't define them, but
 * it doesn't seem to hurt to add them). */
#define	F_OK	0	/* Check for file existence */
/* Well maybe it does hurt.  On newer versions of MSVCRT, an access mode
   of 1 causes invalid parameter error. */
#define	X_OK	1	/* MS access() doesn't check for execute permission. */
#define	W_OK	2	/* Check for write permission */
#define	R_OK	4	/* Check for read permission */
#endif /* _WIN32 */

/* from mingw/include/string.h */
#define strcasecmp(__sz1, __sz2) (_stricmp((__sz1), (__sz2)))

/* from mingw/include/stdint.h */
typedef unsigned   uint32_t;

#ifdef __GNUC__
#define NORETURN __attribute__((__noreturn__))
#else

#if defined(_MSC_VER)
#define NORETURN __declspec(noreturn)
#else
#define NORETURN
#endif /* _MSC_VER */

#ifndef __attribute__
#define __attribute__(x)
#endif
#define __CRT_INLINE
#endif

extern int error(const char * format, ...);
extern NORETURN void die(const char *err, ...) __attribute__((format (printf, 1, 2)));

#ifndef HAVE_STRCHRNUL
#define strchrnul gitstrchrnul
static inline char *gitstrchrnul(const char *s, int c)
{
	while (*s && *s != c)
		s++;
	return (char *)s;
}
#endif

extern time_t tm_to_time_t(const struct tm *tm);

extern void release_pack_memory(size_t, int);

extern char *xstrdup(const char *str);
extern void *xmalloc(size_t size);
extern void *xrealloc(void *ptr, size_t size);
extern void *xcalloc(size_t nmemb, size_t size);
extern ssize_t xread(int fd, void *buf, size_t len);

#ifdef NO_STRLCPY
#define strlcpy gitstrlcpy
extern size_t gitstrlcpy(char *, const char *, size_t);
#endif

#ifdef _WIN32
#define snprintf _snprintf
#endif

#ifdef NO_MMAP

#ifndef PROT_READ
#define PROT_READ 1
#define PROT_WRITE 2
#define MAP_PRIVATE 1
#define MAP_FAILED ((void*)-1)
#endif

#define mmap git_mmap
#define munmap git_munmap
extern void *git_mmap(void *start, size_t length, int prot, int flags, int fd, off_t offset);
extern int git_munmap(void *start, size_t length);

/* This value must be multiple of (pagesize * 2) */
#define DEFAULT_PACKED_GIT_WINDOW_SIZE (1 * 1024 * 1024)

#else /* NO_MMAP */

#include <sys/mman.h>

/* This value must be multiple of (pagesize * 2) */
#define DEFAULT_PACKED_GIT_WINDOW_SIZE \
	(sizeof(void*) >= 8 \
		?  1 * 1024 * 1024 * 1024 \
		: 32 * 1024 * 1024)

#endif /* NO_MMAP */

#ifdef NO_PREAD
#define pread git_pread
extern ssize_t git_pread(int fd, void *buf, size_t count, off_t offset);
#endif
/*
 * Forward decl that will remind us if its twin in cache.h changes.
 * This function is used in compat/pread.c.  But we can't include
 * cache.h there.
 */
extern ssize_t read_in_full(int fd, void *buf, size_t count);

#ifdef _WIN32
#include "../compat/mingw.h"
#endif

/* some defines not available on osx */
#ifndef MAX_PATH
#define MAX_PATH PATH_MAX
#endif

#ifndef BOOL
#define BOOL int
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef UINT
#define UINT unsigned int
#endif

#ifdef _WIN32
#define PATH_SEPERATOR '\\'
#endif

/* default PATH_SEPERATOR is / */
#ifndef PATH_SEPERATOR
#define PATH_SEPERATOR '/'
#endif

#endif
