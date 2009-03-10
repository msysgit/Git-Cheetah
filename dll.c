#include <shlobj.h>
#include <stdio.h>
#include "dll.h"
#include "menuengine.h"
#include "ext.h"
#include "factory.h"
#include "systeminfo.h"
#include "registry.h"

/*
 * The following is just the necessary infrastructure for having a .dll
 * which can be registered as a COM object.
 */
static HINSTANCE hInst;

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

BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved)
{
	hInst = instance;

	if (reason == DLL_PROCESS_ATTACH) {
		object_count = lock_count = 0;
		DisableThreadLibraryCalls(instance);
	}

	return 1;
}

/* replaces a substring pattern with a string replacement within a string
   the replacement occurs in-place, hence string must be large enough to
   hold the result

   the function does not handle recursive replacements, e.g.
     strreplace ("foo", "bar", "another bar");

   always returns *string
*/
static char *strreplace(char *string, const size_t size,
			const char *pattern, const char *replacement)
{
	size_t len = strlen(string);
	const size_t pattern_len = strlen(pattern);
	const size_t replace_len = strlen(replacement);

	char *found = strstr(string, pattern);

	while (found) {
		/* if the new len is greater than size, bail out */
		if (len + replace_len - pattern_len >= size)
			return string;

		if (pattern_len != replace_len)
			memmove(found + replace_len,
			        found + pattern_len,
				len - (found - string) - pattern_len + 1);
		memcpy(found, replacement, replace_len);
		len += replace_len - pattern_len;

		found = strstr(string, pattern);
	}

	return string;
}

/*
 * The following is the data for our minimal regedit engine,
 * required for registration/unregistration of the extension
 */
#define CLASS_CHEETAH CLASSES_ROOT "CLSID\\@@CLSID@@"
#define CONTEXTMENUHANDLER "shellex\\ContextMenuHandlers\\@@PROGRAM_NAME@@"
#define COLUMNHANDLER "shellex\\ColumnHandlers"

static const char *get_module_filename() {
	static char module_filename[MAX_PATH] = { '\0' };

	if (!*module_filename) {
		DWORD module_size;

		module_size = GetModuleFileName(hInst,
				module_filename, MAX_PATH);
		if (0 == module_size)
			return NULL;
	}

	return module_filename;
}

/* as per "How to: Convert Between System::Guid and _GUID" */
static const char *get_class_id()
{
	static char class_id[MAX_REGISTRY_PATH] = { '\0' };

	if (!*class_id) {
		GUID guid = CLSID_git_shell_ext;
		sprintf(class_id,
			"{%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}",
			guid.Data1, guid.Data2, guid.Data3,
			guid.Data4[0], guid.Data4[1], guid.Data4[2],
			guid.Data4[3], guid.Data4[4], guid.Data4[5],
			guid.Data4[6], guid.Data4[7]);
	}

	return class_id;
}

/*
 * Tries to find msysGit in the following order:
 * .. and ../.. (relative to the module)
 * %PATH%
 * as qgit (via InstallLocation of Git)
 SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\Git_is1\InstallLocation
 */
static const reg_value registry_info[] = {
	{ CURRENT_WINDOWS APPROVED_EXT, "@@CLSID@@", "@@PROGRAM_NAME@@" },
	{ CURRENT_WINDOWS APPROVED_EXT "\\@@CLSID@@",
		NULL, NULL },
	{ CURRENT_WINDOWS APPROVED_EXT "\\@@CLSID@@",
		NULL,"@@PROGRAM_NAME@@" },
	{ CLASS_CHEETAH, NULL, NULL },
	{ CLASS_CHEETAH, NULL, "@@PROGRAM_NAME@@" },
	{ CLASS_CHEETAH "\\InProcServer32", NULL, NULL },
	{ CLASS_CHEETAH "\\InProcServer32", NULL, "@@PROGRAM_PATH@@"},
	{ CLASS_CHEETAH "\\InProcServer32", "ThreadingModel", "Apartment" },
	{ CLASSES_ROOT "*\\" CONTEXTMENUHANDLER, NULL, NULL },
	{ CLASSES_ROOT "*\\" CONTEXTMENUHANDLER, NULL, "@@CLSID@@" },
	{ CLASSES_ROOT "Directory\\" CONTEXTMENUHANDLER, NULL, NULL },
	{ CLASSES_ROOT "Directory\\" CONTEXTMENUHANDLER, NULL, "@@CLSID@@" },
	{ CLASSES_ROOT "Directory\\Background\\" CONTEXTMENUHANDLER,
		NULL, NULL },
	{ CLASSES_ROOT "Directory\\Background\\" CONTEXTMENUHANDLER,
		NULL, "@@CLSID@@" },
	{ CLASSES_ROOT "Drive\\" CONTEXTMENUHANDLER, NULL, NULL },
	{ CLASSES_ROOT "Drive\\" CONTEXTMENUHANDLER, NULL, "@@CLSID@@"},
	{ CLASSES_ROOT "Folder\\" CONTEXTMENUHANDLER, NULL, NULL },
	{ CLASSES_ROOT "Folder\\" CONTEXTMENUHANDLER, NULL, "@@CLSID@@" },
	{ CLASSES_ROOT "Folder\\" COLUMNHANDLER "\\@@CLSID@@", NULL, NULL },
	{ CLASSES_ROOT "InternetShortcut\\" CONTEXTMENUHANDLER,
		NULL, NULL },
	{ CLASSES_ROOT "InternetShortcut\\" CONTEXTMENUHANDLER,
		NULL, "@@CLSID@@" },
	{ GIT_CHEETAH_REG_PATH, NULL, NULL },
	{ GIT_CHEETAH_REG_PATH,
		GIT_CHEETAH_REG_PATHTOMSYS, "@@MSYSGIT_PATH@@" },
	{ NULL, NULL, NULL }
};

static const reg_value debug_info[] = {
	{ CURRENT_WINDOWS "Explorer", "DesktopProcess", "1" },
	{ CURRENT_WINDOWS "Explorer\\AlwaysUnloadDll", NULL, NULL },
	{ CURRENT_WINDOWS "Explorer\\Advanced", "SeparateProcess", "1" },
	{ NULL, NULL, NULL }
};

static char msysgit[MAX_PATH] = { '\0' };

static BOOL find_msysgit_in_path()
{
	char *file; /* file part of the path to git.exe */
	DWORD dwFound; /* length of path to git.exe */

	dwFound = SearchPath(NULL, "git.exe", NULL, MAX_PATH, msysgit, &file);
	/* if git is not in the PATH or its path is too long */
	if (0 == dwFound ||
		dwFound > MAX_PATH)
		return FALSE;

	/*
	 * git.exe is in "\bin\" from what we really need
	 * the minimal case we can handle is c:\bin\git.exe
	 * otherwise declare failure
	 */
	if (file < msysgit + 7)
		return FALSE;
	if (strnicmp(file - 5, "\\bin\\", 5))
		return FALSE;
	file[-5] = '\0';

	return TRUE;
}

static BOOL find_msysgit_relative(const char *path)
{
	char *c;

	strcpy(msysgit, get_module_filename());
	c = strrchr(msysgit, '\\');
	c[1] = '\0';
	strcat(msysgit, path);
	strcat(msysgit, "\\bin\\git.exe");
	if (INVALID_FILE_ATTRIBUTES == GetFileAttributes(msysgit)) {
		msysgit[0] = '\0'; /* restore the original result */
		return FALSE;
	}

	c[1] = '\0';
	strcat(msysgit, path);
	return TRUE;
}

static BOOL find_msysgit_uninstall(HKEY root)
{
	HKEY key;
	HRESULT result;
	DWORD valuelen = MAX_PATH;

	result = RegOpenKeyEx(root,
			CURRENT_WINDOWS "\\Uninstall\\Git_is1",
			0, KEY_READ, &key);
	if (ERROR_SUCCESS != result)
		return FALSE;

	result = RegQueryValue(key, "InstallLocation",
			(LPBYTE)msysgit, &valuelen);
	return ERROR_SUCCESS == result;
}

static HKEY setup_root;

static const char *find_msysgit()
{
	if ('\0' == msysgit[0]) {
		if (find_msysgit_relative(".."))
			return msysgit;

		if (find_msysgit_relative("..\\.."))
			return msysgit;

		if (find_msysgit_in_path())
			return msysgit;

		if (setup_root)
			find_msysgit_uninstall(setup_root);
	}

	return msysgit;
}

/*
 * required by registry.c
 * supports @@PROGRAM_NAME@@, @@PROGRAM_PATH@@, @@CLSID@@ patterns
 */
char *get_registry_path(const char *src, char dst[MAX_REGISTRY_PATH])
{
	if (NULL == src)
		return NULL;

	strcpy(dst, src);
	strreplace(dst, MAX_REGISTRY_PATH,
			"@@PROGRAM_NAME@@", program_name);
	strreplace(dst, MAX_REGISTRY_PATH,
			"@@PROGRAM_PATH@@", get_module_filename());
	strreplace(dst, MAX_REGISTRY_PATH,
			"@@CLSID@@", get_class_id());
	strreplace(dst, MAX_REGISTRY_PATH,
			"@@MSYSGIT_PATH@@", find_msysgit());

	return dst;
}

HRESULT PASCAL DllRegisterServer(void)
{
	setup_root = HKEY_CURRENT_USER;
	return create_reg_entries (setup_root, registry_info);
}

HRESULT PASCAL DllUnregisterServer(void)
{
	setup_root = HKEY_CURRENT_USER;
	return delete_reg_entries(setup_root, registry_info);
}

/* provide means to create/delete keys:
   - described in Debugging with The Shell;
   - machine-wide registration and debugging info

   possible combinations of regsvr32 command line options:
             -n  (absent)               (present)
   -i:
   (absent)      user reg               (invalid)
   debug         user reg+debug         user debug
   machine       user+machine reg       machine reg
   machinedebug  user+machine reg+debug machine reg+debug

   Obviously missing option is "machine debug". To accomplish:
   - execute "regsvr32 -n -i:machinedebug" and then
   - regsvr32 -u -n -i:machine
*/
HRESULT PASCAL DllInstall(BOOL bInstall, LPCWSTR pszCmdLine)
{
	BOOL bDebug = NULL != wcsstr(pszCmdLine, L"debug");
	HRESULT result = ERROR_SUCCESS;

	setup_root = wcsstr(pszCmdLine, L"machine") ?
			HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER;

	if (bInstall) {
		if (bDebug)
			result = create_reg_entries(setup_root, debug_info);

		/* for user-only registration, use DllRegister */
		if (ERROR_SUCCESS == result &&
		    HKEY_LOCAL_MACHINE == setup_root)
			result = create_reg_entries(setup_root,
					registry_info);
	} else { /* uninstall */
		if (bDebug)
			result = delete_reg_entries(setup_root, debug_info);

		/* for user-only unregistration, use DllUnregister */
		if (ERROR_SUCCESS == result &&
		    HKEY_LOCAL_MACHINE == setup_root)
			result = delete_reg_entries(setup_root,
					registry_info);
	}

	return result;
}
