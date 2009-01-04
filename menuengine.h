#ifndef MENUENGINE_H
#define MENUENGINE_H

struct git_data {
	struct git_shell_ext {
		void *virtual_table;
		struct git_data *git_data;
	} shell_ext;
	struct git_menu {
		void *virtual_table;
		struct git_data *git_data;
	} menu;
	unsigned int count;
	char name[MAX_PATH];
};

#endif /* MENUENGINE_H */
