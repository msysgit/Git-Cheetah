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
	struct git_columns {
		void *virtual_table;
		struct git_data *git_data;
	} columns;
	unsigned int count;
	char name[MAX_PATH];
	struct strbuf other_files;
};

/*
 * flags to match the selection
 */
#define MENU_ITEM_ALWAYS 0		/* always insert the menu item */
#define MENU_ITEM_CLEANUP (1 << 31)	/* menu item requires free() of
					   string and helptext */
#define MENU_ITEM_LAST -1		/* the last menu item */

struct menu_item;

typedef unsigned int selection_to_mask(struct git_data *);
typedef void menu_item_handler(struct git_data *, unsigned int);
/*
 * if platform-specific builder returns TRUE, the menu item
 * is added to the active menu and can be passed to menu_item_handler
 * later, e.g. when a user selects the item
 */
typedef BOOL menu_item_builder(struct git_data *, const struct menu_item *, void *);

struct menu_item {
	unsigned int selection;
	char *string;
	char *helptext;
	menu_item_builder *builder;
	menu_item_handler *handler;
};

extern struct menu_item *active_menu;
extern unsigned int next_active_item;

/*
 * The main entry point of the menu engine.
 *
 * Important things to note:
 * - it resets the active menu;
 * - it walks menu_def until MENU_ITEM_LAST is found or
 *   menu_def_count times, whatever is earlier.
 */
void build_menu_items(struct git_data *data,
		      selection_to_mask *mask_builder,
		      const struct menu_item menu_def[],
		      const unsigned int menu_def_count,
		      void *platform);

void reset_active_menu();

char *get_menu_item_text(unsigned int id);

/*
 * Parses a menu entries text and removes the '&' character
 * which is used to specify the shortcut key. It returns
 * the first found key or zero if nothing is found
 */
static inline int parse_and_remove_shortcuts(char *name)
{
	int i,j;
	char key = 0;
	for (i=0,j=0; name[i] && name[j]; i++,j++) {
		if (!key && name[j] == '&') {
			key = name[j+1];
			j++;
		}
		name[i] = name[j];
	}
	name[i] = '\0';
	return key;
}

void handle_menu_item(void *data, unsigned int id);

/*
 * usually, menu engine will append items, based on the return
 * from menu_item_builder, but a custom builder might need to
 * instruct the engine to track a particular item
 */
void append_active_menu(const struct menu_item *item);

#endif /* MENUENGINE_H */
