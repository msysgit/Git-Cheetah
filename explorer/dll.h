#ifndef DLL_H
#define DLL_H

#define GIT_CHEETAH_REG_PATH "SOFTWARE\\Git-Cheetah"
#define GIT_CHEETAH_REG_PATHTOMSYS "PathToMsys"

extern const char *program_name;
extern const char *program_version;
extern const char *program_id;

STDAPI DllGetClassObject(REFCLSID obj_guid, REFIID factory_guid,
				 void **factory_handle);
STDAPI DllCanUnloadNow(void);
STDAPI DllRegisterServer(void);
STDAPI DllUnregisterServer(void);
BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved);

#endif /* DLL_H */
