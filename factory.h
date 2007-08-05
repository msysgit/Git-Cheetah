#ifndef FACTORY_H
#define FACTORY_H

STDMETHODIMP class_factory_query_interface(IClassFactory *this,
					   REFIID guid, void **pointer);

IClassFactoryVtbl factory_virtual_table;
IClassFactory factory;

#endif /* FACTORY_H */
