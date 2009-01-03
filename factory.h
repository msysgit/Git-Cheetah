#ifndef FACTORY_H
#define FACTORY_H

STDMETHODIMP class_factory_query_interface(IClassFactory *this,
					   REFIID guid, void **pointer);

extern IClassFactoryVtbl factory_virtual_table;
extern IClassFactory factory;

#endif /* FACTORY_H */
