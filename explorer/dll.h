#ifndef DLL_H
#define DLL_H

extern const char *program_name;
extern const char *program_version;
extern const char *program_id;

HRESULT PASCAL DllGetClassObject(REFCLSID obj_guid, REFIID factory_guid,
				 void **factory_handle);
HRESULT PASCAL DllCanUnloadNow(void);
HRESULT PASCAL DllRegisterServer(void);
HRESULT PASCAL DllUnregisterServer(void);
BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved);

#endif /* DLL_H */
