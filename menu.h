#ifndef MENU_H
#define MENU_H

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
} git_menu_virtual_table;

#endif /* MENU_H */
