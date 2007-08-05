#ifndef DLL_H
#define DLL_H

const char *program_name = "Git-Cheetah";
const char *program_version = "Git-Cheetah.Application.1";
const char *program_id = "Git-Cheetah.Application";

HRESULT PASCAL DllGetClassObject(REFCLSID obj_guid, REFIID factory_guid,
				 void **factory_handle);
HRESULT PASCAL DllCanUnloadNow(void);
HRESULT PASCAL DllRegisterServer(void);
HRESULT PASCAL DllUnregisterServer(void);
BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved);

#endif /* DLL_H */
