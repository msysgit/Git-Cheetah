/* util.c
 *
 * Implementation of a finder menu plugin for git-cheetah
 *
 * This provides Mac OS X specific utility functions
 *
 * Copyright (C) Heiko Voigt, 2009
 *
 * inspired by an example from Brent Simmons
 * brent@ranchero.com, http://macte.ch/kmyXM
 *
 */
#include "../common/git-compat-util.h"
#include <dirent.h>
#include <unistd.h>
#include <sys/wait.h>
#include "../common/strbuf.h"
#include "../common/debug.h"
#include "../common/exec.h"
#include "plugin.h"
#include "util.h"

#define MAX_ARGS 32

const char *git_path()
{
	return "";
}

void message_box(const char *string)
{
	CFStringRef msg = CFStringCreateWithCString(NULL,
			string, kCFStringEncodingASCII);

	CFUserNotificationDisplayNotice(0,
			kCFUserNotificationNoteAlertLevel,
			NULL, NULL, NULL, CFSTR("Git Cheetah says"),
			msg, CFSTR("Ok"));
}

int selection_to_path(char *path, int len, const AEDesc *selection)
{
	AEDesc selected_item = { typeNull, NULL };
	int status = 1;
	Size item_size;

	AliasHandle item_data_handle;
	FSRef file_ref;
	Boolean changed = false;

	if ((!get_file_from_selection(selection, &selected_item)) &&
	    (!get_file_from_selection_list(selection, &selected_item))) {
		debug_git("failure get_file_from_*");
	   return 0;
	}

	item_size = AEGetDescDataSize(&selected_item);

	item_data_handle = (AliasHandle) NewHandle(item_size);
	if (item_data_handle == nil) {
		status = 0;
		goto selection_to_path_fail;
	}

	if (AEGetDescData(&selected_item, *item_data_handle, item_size) != noErr) {
		status = 0;
		goto selection_to_path_fail;
	}

	if (FSResolveAlias (NULL, item_data_handle, &file_ref, &changed) != noErr) {
		status = 0;
		goto selection_to_path_fail;
	}

	if (FSRefMakePath(&file_ref, (UInt8 *) path, len) != noErr)
		status = 0;

selection_to_path_fail:
	DisposeHandle((Handle) item_data_handle);
	AEDisposeDesc(&selected_item);
	return status;
}


/* if this selection is a file return it, if not NULL is returned */
AEDesc *get_file_from_selection(const AEDesc *selection, AEDesc *file)
{
	AEDesc temp = { typeNull, NULL };

	if (selection->descriptorType == typeAlias) {
		if (AEDuplicateDesc(selection, file) != noErr)
			return NULL;
		return file;
	}

	if (AECoerceDesc(selection, typeAlias, &temp) != noErr)
		goto get_file_from_selection_fail;

	if (temp.descriptorType != typeAlias)
		goto get_file_from_selection_fail;

	if (AEDuplicateDesc(&temp, file) != noErr)
		goto get_file_from_selection_fail;

	/* TODO: check if this is a memory leak */
	return file;

get_file_from_selection_fail:
	AEDisposeDesc(&temp);
	return NULL;
}

AEDesc *get_file_from_selection_list(const AEDesc *selection, AEDesc *file
		/* , int *index */)
{
	AEDesc temp = { typeNull, NULL };
	AEDesc *result;
	AEKeyword dummy;
	long count;

	if (selection->descriptorType != typeAEList)
		return NULL;

	if (AECountItems(selection, &count) != noErr)
		return NULL;

	if (count != 1)
		return NULL;

	if (AEGetNthDesc(selection, 1, typeWildCard, &dummy, &temp) != noErr)
		return NULL;

	result = get_file_from_selection(&temp, file);
	AEDisposeDesc(&temp);

	return result;
}
