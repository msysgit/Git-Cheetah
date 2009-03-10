#include <shlobj.h>
#include "menuengine.h"
#include "factory.h"
#include "ext.h"
#include "menu.h"
#include "columns.h"

/*
 * Since COM objects cannot be constructed like your traditional object (i.e.
 * with a proper constructor), they have to be constructed by another object,
 * the class factory.
 *
 * The class factory is an object which exists exactly once, and it cannot
 * be constructed or destroyed.  Its sole purpose is to construct objects
 * given an interface.
 */

STDMETHODIMP class_factory_query_interface(IClassFactory *this, 
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
	data->columns.virtual_table = &git_columns_virtual_table;
	data->shell_ext.git_data = data->menu.git_data =
		data->columns.git_data = data;

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

IClassFactoryVtbl factory_virtual_table = {
	class_factory_query_interface,
	return_one,
	return_one,
	create_instance,
	lock_server
};

IClassFactory factory = {
	&factory_virtual_table
};
