#include <shlobj.h>
#include <stdarg.h>
#include <tchar.h>
#include "menuengine.h"
#include "ext.h"
#include "debug.h"
#include "systeminfo.h"

DWORD object_count = 0;
DWORD lock_count = 0;

/*
 * Standard methods for the IUnknown interface:
 * add_ref, release, query_interface and initialize.
 *
 * Both of our COM objects contain pointers to the git_data object.
 */

inline ULONG STDMETHODCALLTYPE add_ref_git_data(struct git_data *this_)
{
	return ++(this_->count);
}

inline ULONG STDMETHODCALLTYPE release_git_data(struct git_data *this_)
{
	if (--(this_->count) == 0) {
		GlobalFree(this_);
		InterlockedDecrement(&object_count);
		return 0;
	}
	return this_->count;
}

inline STDMETHODIMP query_interface_git_data(struct git_data *this_,
					     REFIID iid, LPVOID FAR *pointer)
{
	if (IsEqualIID(iid, &IID_git_shell_ext) ||
			IsEqualIID(iid, &IID_IShellExtInit) ||
			IsEqualIID(iid, &IID_IUnknown)) {
		*pointer = &this_->shell_ext;
	} else if (IsEqualIID(iid, &IID_git_menu) ||
			IsEqualIID(iid, &IID_IContextMenu)) {
		*pointer = &this_->menu;
	} else if (IsEqualIID(iid, &IID_git_columns) ||
			IsEqualIID(iid, &IID_IColumnProvider)) {
		*pointer = &this_->columns;
	} else
		return E_NOINTERFACE;

	add_ref_git_data(this_);
	return NOERROR;
}

inline STDMETHODIMP initialize_git_data(struct git_data *this_,
					LPCITEMIDLIST folder,
					LPDATAOBJECT data, HKEY id)
{
	FORMATETC format
		= {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
	STGMEDIUM stg = {TYMED_HGLOBAL};
	HDROP drop;
	UINT count;
	HRESULT result = S_OK;

	/* if we can't find msysPath, don't even try to do anything else */
	if (!msys_path())
		return E_NOTIMPL;

	/* store the folder, if provided */
	if (folder)
		SHGetPathFromIDList(folder, this_->name);

	if (!data)
		return S_OK;

	if (FAILED(data->lpVtbl->GetData(data, &format, &stg)))
		return E_INVALIDARG;

	drop = (HDROP)GlobalLock(stg.hGlobal);

	if (!drop)
		return E_INVALIDARG;

	count = DragQueryFile(drop, 0xFFFFFFFF, NULL, 0);

	if (count == 0)
		result = E_INVALIDARG;
	else if (!DragQueryFile(drop, 0, this_->name, sizeof(this_->name)))
		result = E_INVALIDARG;

	GlobalUnlock(stg.hGlobal);
	ReleaseStgMedium(&stg);

	return result;
}

/*
 * Define the shell extension.
 *
 * Its sole purpose is to be able to query_interface() the context menu.
 */
DEFINE_STANDARD_METHODS(git_shell_ext)

struct git_shell_ext_virtual_table git_shell_ext_virtual_table = {
	query_interface_git_shell_ext,
	add_ref_git_shell_ext,
	release_git_shell_ext,
	initialize_git_shell_ext
};
