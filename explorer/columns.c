
#include "../common/cache.h"
#include "../common/exec.h"

#include <shlobj.h>
#include "../common/menuengine.h"
#include "ext.h"
#include "columns.h"

#ifndef _MSC_VER
/*
 * this flag, not defined in mingw's ShlObj.h, is a hint that the file
 * has changed since the last call to GetItemData
 */
#define SHCDF_UPDATEITEM        0x00000001

#ifndef _WIN64
/* convenience macro, not defined in mingw's OleAuto.h */
#define V_I1REF(X)       V_UNION(X, pcVal)
#endif /* _WIN64 */
#endif

#define CHEETAH_COLUMN_FLAGS (SHCOLSTATE_TYPE_STR)

/*
 * If a non-standard FMT is specified, the column will be accessible
 * only in repo folders. But if FMTID_SummaryInformation is used,
 * the column's info will be "merged" into CHEETAH_PID_* and may
 * be visible in the Details on folder's task pane
 */
#define CHEETAH_FMTID IID_git_columns

/*
 * column's id, that can be matched to one of standard FMTs
 * for example, if FMTID is FMTID_SummaryInformation than
 * PID can be PIDSI_AUTHOR, so the status is provided in the
 * Author column and visible in the Details
 */
#define CHEETAH_STATUS_PID 0

/* names and descriptions MUST be wchar * */
#define CHEETAH_STATUS_NAME L"Git Status"
#define CHEETAH_STATUS_DESC L"Status of the file in a Git repository"

STDMETHODIMP initialize_columns(void *p, LPCSHCOLUMNINIT psci)
{
	struct git_data *this_ = ((struct git_columns *)p)->git_data;
	int status;

	wcstombs(this_->name, psci->wszFolder, MAX_PATH);

	/*
	 * don't try to do anything about cache here, because
	 * we're called even if get_item_data will not be called
	 */
	status = exec_program(this_->name, NULL, NULL, WAITMODE,
		"git", "rev-parse", "--show-prefix", NULL);

	/*
	 * if something went terribly wrong or not a repo,
	 * return E_FAIL to prevent calls to get_column_info
	 */
	return status ? E_FAIL : S_OK;
}

STDMETHODIMP get_column_info(void *p, DWORD dwIndex, SHCOLUMNINFO *psci)
{
	struct git_data *this_ = ((struct git_columns *)p)->git_data;

	/* do NOT FORGET to increase this when adding more columns */
	if (0 < dwIndex)
		return S_FALSE;

	psci->scid.fmtid = CHEETAH_FMTID;
	psci->scid.pid = CHEETAH_STATUS_PID;
	psci->vt = VT_LPSTR;
	psci->fmt = LVCFMT_LEFT;
	psci->cChars = 15;
	psci->csFlags = CHEETAH_COLUMN_FLAGS;

	lstrcpynW(psci->wszTitle,
		CHEETAH_STATUS_NAME, MAX_COLUMN_NAME_LEN);
	lstrcpynW(psci->wszDescription,
		CHEETAH_STATUS_DESC, MAX_COLUMN_DESC_LEN);

	return S_OK;
}

static int cache_others(char *wd, struct strbuf *cache)
{
      int status;

      if (cache->alloc)
	      return 0;

      strbuf_init(cache, 0);
      status = exec_program(wd, cache, NULL, WAITMODE,
	      "git", "ls-files", "-z", "--others", "--exclude-standard",
	      NULL);

      /* something went terribly wrong or not a repo */
      if (status)
	      strbuf_release(cache);
      return status;
}

/*
 * cache files' status, and return non-zero on the first failure
 * because if something goes wrong, there is no need to try other statuses
 */
static int cache(struct git_data *this_)
{
	int status;
	if (status = cache_others(this_->name, &this_->other_files))
		return status;

	/* if everything succeeded, return success */
	return 0;
}

static char *parse_others(struct strbuf *cache, char *file)
{
	char *start = cache->buf;
	while (*start) {
		if (!stricmp(file, start))
			return "other";

		start += strlen(start) + 1;
	}

	return 0;
}

static char *parse_status(struct git_data *this_, char *file)
{
	 char *result = NULL;

	 if (result = parse_others(&this_->other_files, file))
		 return result;

	/* if no status was found, return NULL string */
	return NULL;
}


STDMETHODIMP get_item_data(void *p,
			   LPCSHCOLUMNID pscid,
			   LPCSHCOLUMNDATA pscd,
			   VARIANT *pvarData)
{
	struct git_data *this_ = ((struct git_columns *)p)->git_data;
	char file_path[MAX_PATH], *file = file_path;
	char *result;

	/* directories don't have status */
	if (pscd->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		return S_FALSE;

	/* update cache and bail out on failure */
	if (cache(this_))
		return S_FALSE;

	if (pscd->dwFlags & SHCDF_UPDATEITEM) {
		/*
		 * we can add the file to the usual ls-files
		 * or we can refresh the whole cache
		 */
	}

	wcstombs(file, pscd->wszFile, MAX_PATH);
	file += strlen(this_->name) + 1;

	/* get the status from cache and return if any */
	if (result = parse_status(this_, file)) {
		V_VT(pvarData) = VT_LPSTR;
		V_I1REF(pvarData) = result;
		return S_OK;
	}

	return S_FALSE;
}

DEFINE_STANDARD_METHODS(git_columns)

struct git_columns_virtual_table git_columns_virtual_table = {
	query_interface_git_columns,
	add_ref_git_columns,
	release_git_columns,
	initialize_columns,
	get_column_info,
	get_item_data
};
