#include <shlobj.h>

/*
 * Some class IDs.  Let's hope they are unique enough.
 */

static const IID LIBID = {
	0xca586c7f, 0x7c84, 0x4b88,
	{0x85, 0x37, 0x72, 0x67, 0x24, 0xdf, 0x69, 0x29}
};

static const CLSID CLSID_git_shell_ext = {
	0xca586c80, 0x7c84, 0x4b88,
	{0x85, 0x37, 0x72, 0x67, 0x24, 0xdf, 0x69, 0x29}
};

static const IID IID_git_shell_ext = {
	0xca586c81, 0x7c84, 0x4b88,
	{0x85, 0x37, 0x72, 0x67, 0x24, 0xdf, 0x69, 0x29}
};

static const CLSID CLSID_git_menu = {
	0xca586c82, 0x7c84, 0x4b88,
	{0x85, 0x37, 0x72, 0x67, 0x24, 0xdf, 0x69, 0x29}
};

static const IID IID_git_menu = {
	0xca586c83, 0x7c84, 0x4b88,
	{0x85, 0x37, 0x72, 0x67, 0x24, 0xdf, 0x69, 0x29}
};

struct git_data {
	struct git_shell_ext {
		struct git_shell_ext_virtual_table *virtual_table;
		struct git_data *git_data;
	} shell_ext;
	struct git_menu {
		struct git_menu_virtual_table *virtual_table;
		struct git_data *git_data;
	} menu;
	unsigned int count;
	char name[MAX_PATH];
};

struct git_shell_ext_virtual_table
{
	STDMETHOD(query_interface)(void *, REFIID, PVOID*);
	STDMETHOD_(ULONG, add_ref)(void *);
	STDMETHOD_(ULONG, release)(void *);
	STDMETHOD(initialize)(void *,
			LPCITEMIDLIST, LPDATAOBJECT, HKEY);
};

struct git_menu_virtual_table
{
        STDMETHOD(query_interface)(void *, REFIID, PVOID*);
        STDMETHOD_(ULONG, add_ref)(void *);
        STDMETHOD_(ULONG, release)(void *);
	STDMETHOD(query_context_menu)(void *,
			HMENU, UINT, UINT, UINT, UINT);
        STDMETHOD(invoke_command)(void *, LPCMINVOKECOMMANDINFO);
	STDMETHOD(get_command_string)(void *,
			UINT, UINT, PUINT, LPSTR, UINT);
};

