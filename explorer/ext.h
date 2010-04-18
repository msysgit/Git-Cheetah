#ifndef EXT_H
#define EXT_H

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

// {62871171-DCED-4caf-8483-952EE3D610F9}
static const IID IID_git_columns = {
	0x62871171, 0xdced, 0x4caf,
	{0x84, 0x83, 0x95, 0x2e, 0xe3, 0xd6, 0x10, 0xf9}
};

extern struct git_shell_ext_virtual_table
{
	STDMETHOD(QueryInterface)(void *, REFIID, PVOID*);
	STDMETHOD_(ULONG, add_ref)(void *);
	STDMETHOD_(ULONG, release)(void *);
	STDMETHOD(initialize)(void *,
			LPCITEMIDLIST, LPDATAOBJECT, HKEY);
} git_shell_ext_virtual_table;

#define DEFINE_STANDARD_METHODS(name) \
	static ULONG STDMETHODCALLTYPE add_ref_##name(void *p) { \
	debug_git("add_ref_##name"); \
		struct name *this_ = p; \
		return add_ref_git_data(this_->git_data); \
	} \
	\
	static ULONG STDMETHODCALLTYPE \
			release_##name(void *p) { \
	debug_git("release_##name"); \
		struct name *this_ = p; \
		return release_git_data(this_->git_data); \
	} \
	\
	static STDMETHODIMP query_interface_##name(void *p, \
			REFIID iid, LPVOID FAR *pointer) { \
	debug_git("query_interface_##name"); \
		struct name *this_ = p; \
		return query_interface_git_data(this_->git_data, \
				iid, pointer); \
	} \
	\
	static STDMETHODIMP initialize_##name(void *p, \
			LPCITEMIDLIST folder, LPDATAOBJECT data, HKEY id) { \
	debug_git("initialize_##name"); \
		struct name *this_ = p; \
		return initialize_git_data(this_->git_data, folder, data, id); \
	}

extern DWORD object_count;
extern DWORD lock_count;

inline STDMETHODIMP query_interface_git_data(struct git_data *this_,
					     REFIID iid, LPVOID FAR *pointer);
inline ULONG STDMETHODCALLTYPE add_ref_git_data(struct git_data *this_);
inline ULONG STDMETHODCALLTYPE release_git_data(struct git_data *this_);
inline STDMETHODIMP initialize_git_data(struct git_data *this_,
					LPCITEMIDLIST folder,
					LPDATAOBJECT data, HKEY id);

#endif /* EXT_H */
