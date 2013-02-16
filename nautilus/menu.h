#ifndef CHEETAH_NAUTILUS_MENU_H
#define CHEETAH_NAUTILUS_MENU_H

/* menu filler */
GList *git_extension_get_file_items(NautilusMenuProvider *provider,
		GtkWidget *window,
		GList *files);
GList *git_extension_get_background_items(NautilusMenuProvider *provider,
		GtkWidget *window,
		NautilusFileInfo *current_folder);

/* command callback */
void invoke_command(NautilusMenuItem *item, gpointer user_data);

#endif /* CHEETAH_NAUTILUS_MENU_H */
