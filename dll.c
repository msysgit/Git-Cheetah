#include <shlobj.h>
#include "dll.h"
#include "ext.h"
#include "factory.h"

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
