#ifndef COLUMNS_H
#define COLUMNS_H

extern struct git_columns_virtual_table {
	STDMETHOD(query_interface)(void *, REFIID, PVOID*);
	STDMETHOD_(ULONG, add_ref)(void *);
	STDMETHOD_(ULONG, release)(void *);

	STDMETHOD(initialize_columns)(void *, LPCSHCOLUMNINIT);
	STDMETHOD(get_column_info)(void *,
			DWORD dwIndex, SHCOLUMNINFO *psci);
	STDMETHOD(get_item_data)(void *,
			LPCSHCOLUMNID pscid, LPCSHCOLUMNDATA pscd,
			VARIANT *pvarData);
} git_columns_virtual_table;

#endif /* COLUMNS_H */
