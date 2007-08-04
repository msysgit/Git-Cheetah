#include <stdarg.h>
#include <tchar.h>
#include "git_shell_ext.h"

static DWORD object_count = 0, lock_count = 0;

static void debug(char *format, ...)
{
	va_list params;
	char buffer[1024];

	va_start(params, format);
	vsprintf(buffer, format, params);
	va_end(params);
	MessageBox(0, buffer, "Hello", MB_OK|MB_ICONEXCLAMATION);
}

/*
 * Standard methods for the IUnknown interface:
 * add_ref, release, query_interface and initialize.
 *
 * Both of our COM objects contain pointers to the git_data object.
 */

static inline ULONG STDMETHODCALLTYPE add_ref_git_data(struct git_data *this_)
{
	return ++(this_->count);
}

static inline ULONG STDMETHODCALLTYPE release_git_data(struct git_data *this_)
{
	if (--(this_->count) == 0) {
		GlobalFree(this_);
		InterlockedDecrement(&object_count);
		return 0;
	}
	return this_->count;
}

static inline STDMETHODIMP query_interface_git_data(struct git_data *this_,
		REFIID iid, LPVOID FAR *pointer)
{
	if (IsEqualIID(iid, &IID_git_shell_ext) ||
			IsEqualIID(iid, &IID_IShellExtInit) ||
			IsEqualIID(iid, &IID_IUnknown)) {
		*pointer = this_;
	} else if (IsEqualIID(iid, &IID_git_menu) ||
			IsEqualIID(iid, &IID_IContextMenu)) {
		*pointer = (void *)(((void **)this_) + 1);
	} else
		return E_NOINTERFACE;

	add_ref_git_data(this_);
	return NOERROR;
}

static inline STDMETHODIMP initialize_git_data(struct git_data *this_,
		LPCITEMIDLIST folder, LPDATAOBJECT data, HKEY id)
{
	FORMATETC format
		= {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
	STGMEDIUM stg = {TYMED_HGLOBAL};
	HDROP drop;
	UINT count;
	HRESULT result = S_OK;

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

#define DEFINE_STANDARD_METHODS(name) \
	static ULONG STDMETHODCALLTYPE add_ref_##name(void *p) { \
		struct name *this_ = p; \
		return add_ref_git_data(this_->git_data); \
	} \
	\
	static ULONG STDMETHODCALLTYPE \
			release_##name(void *p) { \
		struct name *this_ = p; \
		return release_git_data(this_->git_data); \
	} \
	\
	static STDMETHODIMP query_interface_##name(void *p, \
			REFIID iid, LPVOID FAR *pointer) { \
		struct name *this_ = p; \
		return query_interface_git_data(this_->git_data, \
				iid, pointer); \
	} \
	\
	static STDMETHODIMP initialize_##name(void *p, \
			LPCITEMIDLIST folder, LPDATAOBJECT data, HKEY id) { \
		struct name *this_ = p; \
		return initialize_git_data(this_->git_data, folder, data, id); \
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

/*
 * These are the functions for handling the context menu.
 */

static STDMETHODIMP query_context_menu(void *p, HMENU menu,
		UINT index, UINT first_command, UINT last_command, UINT flags)
{
	struct git_menu *this_menu = p;
	struct git_data *this_ = this_menu->git_data;
	if (flags & CMF_DEFAULTONLY)
		return MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_NULL, 0);

	InsertMenu(menu, index, MF_STRING | MF_BYPOSITION,
			first_command, _T("SimpleShlExt Test Item"));

	return MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_NULL, 1);
}

static STDMETHODIMP invoke_command(void *p,
		LPCMINVOKECOMMANDINFO info)
{
	struct git_menu *this_menu = p;
	struct git_data *this_ = this_menu->git_data;
	int command = LOWORD(info->lpVerb);
	if (HIWORD(info->lpVerb) != 0)
		return E_INVALIDARG;

	if (command == 0) {
		TCHAR msg[MAX_PATH + 32];

		wsprintf(msg, _T("The selected file was:\n\n%s"), this_->name);
		MessageBox(info->hwnd, msg, _T("SimpleShlExt"),
				MB_ICONINFORMATION);

		return S_OK;
	}

	return E_INVALIDARG;
}

static STDMETHODIMP get_command_string(void *p, UINT id,
		UINT flags, UINT *reserved, LPSTR name, UINT size)
{
	struct git_menu *this_menu = p;
	struct git_data *this_ = this_menu->git_data;
	if (id == 0)
		return E_INVALIDARG;

#if 0
	if (flags & GCS_HELPTEXT) {
		LPCTSTR text = _T("This is the simple shell extension's help");

		if (flags & GCS_UNICODE)
			lstrcpynW((LPWSTR)name, mbrtowc(text), size);
		else
			lstrcpynA(name, text, size);

		return S_OK;
	}
#endif

	return E_INVALIDARG;
}

DEFINE_STANDARD_METHODS(git_menu)

struct git_menu_virtual_table git_menu_virtual_table = {
	query_interface_git_menu,
	add_ref_git_menu,
	release_git_menu,
	query_context_menu,
	invoke_command,
	get_command_string
};

/*
 * Since COM objects cannot be constructed like your traditional object (i.e.
 * with a proper constructor), they have to be constructed by another object,
 * the class factory.
 *
 * The class factory is an object which exists exactly once, and it cannot
 * be constructed or destroyed.  Its sole purpose is to construct objects
 * given an interface.
 */

static STDMETHODIMP class_factory_query_interface(IClassFactory *this, 
		REFIID guid, void **pointer)
{
	if (!IsEqualIID(guid, &IID_IUnknown) &&
			!IsEqualIID(guid, &IID_IClassFactory)) {
		*pointer = 0;
		return E_NOINTERFACE;
	}

	*pointer = this;
	return NOERROR;
}

static ULONG STDMETHODCALLTYPE return_one(IClassFactory *this)
{
	return(1);
}

static STDMETHODIMP create_instance(IClassFactory *this_,
		IUnknown *outer, REFIID guid, void **pointer)
{
	HRESULT result;
	struct git_data *data;

	*pointer = 0;

	if (outer)
		return CLASS_E_NOAGGREGATION;

	if (!(data = GlobalAlloc(GMEM_FIXED, sizeof(struct git_data))))
		return E_OUTOFMEMORY;
	memset(data, 0, sizeof(struct git_data));

	data->shell_ext.virtual_table = &git_shell_ext_virtual_table;
	data->menu.virtual_table = &git_menu_virtual_table;
	data->shell_ext.git_data = data->menu.git_data = data;

	result = query_interface_git_data(data, guid, pointer);
	if (!result)
		InterlockedIncrement(&object_count);
	return result;
}

static STDMETHODIMP lock_server(IClassFactory *this, BOOL lock)
{
	if (lock)
		InterlockedIncrement(&lock_count);
	else
		InterlockedDecrement(&lock_count);

	return NOERROR;
}

static IClassFactoryVtbl factory_virtual_table = {
	class_factory_query_interface,
	return_one,
	return_one,
	create_instance,
	lock_server
};

static IClassFactory factory = {
	&factory_virtual_table
};

/*
 * The following is just the necessary infrastructure for having a .dll
 * which can be registered as a COM object.
 */

HRESULT PASCAL DllGetClassObject(REFCLSID obj_guid, REFIID factory_guid,
		void **factory_handle)
{
	if (IsEqualCLSID(obj_guid, &CLSID_git_shell_ext) ||
			IsEqualCLSID(obj_guid, &CLSID_git_menu))
		return class_factory_query_interface(&factory,
				factory_guid, factory_handle);

	*factory_handle = 0;
	return CLASS_E_CLASSNOTAVAILABLE;
}

HRESULT PASCAL DllCanUnloadNow(void)
{
   return (object_count || lock_count) ? S_FALSE : S_OK;
}

const char *program_name = "Hare GIT";
const char *program_version = "HareGIT.Application.1";
const char *program_id = "HareGIT.Application";

HRESULT PASCAL DllRegisterServer(void)
{
	char module[MAX_PATH];
	wchar_t module_name[MAX_PATH];
	ITypeLib *typelib = NULL;

	GetModuleFileName(NULL, module, MAX_PATH);
	MultiByteToWideChar(CP_ACP, 0, module, -1, module_name, MAX_PATH);
	if (LoadTypeLib(module_name, &typelib) == S_OK) {
		HRESULT result = RegisterTypeLib(typelib, module_name, NULL);
		typelib->lpVtbl->Release(typelib);
		return result;
	}
	return 1;
}

HRESULT PASCAL DllUnregisterServer(void)
{
	return S_OK;
}

BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved)
{
	if (reason == DLL_PROCESS_ATTACH) {
		object_count = lock_count = 0;
		DisableThreadLibraryCalls(instance);
	}

        return 1;
}

