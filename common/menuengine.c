
#include "cache.h"
#include "menuengine.h"

struct menu_item *active_menu;
unsigned int next_active_item;

void reset_active_menu()
{
	if (active_menu) {
		do {
			struct menu_item item;
			next_active_item--;

			item = active_menu[next_active_item];
			if (MENU_ITEM_CLEANUP == item.selection) {
				if (item.string)
					free(item.string);
				if (item.helptext)
					free(item.helptext);
			}

		} while (next_active_item);

		free(active_menu);
	}

	active_menu = 0;
	next_active_item = 0;
}

void append_active_menu(const struct menu_item *item)
{
	active_menu = realloc(active_menu,
		(next_active_item + 1) * sizeof(struct menu_item));
	active_menu[next_active_item] = *item;
	next_active_item++;
}

void build_menu_items(struct git_data *data,
		      selection_to_mask *mask_builder,
		      const struct menu_item menu_def[],
		      const unsigned int menu_def_count,
		      void *platform)
{
	unsigned int i = 0;
	const unsigned int selection = mask_builder(data);

	reset_active_menu();

	if (MENU_ITEM_LAST == selection)
		return;

	for (i = 0;
	     i < menu_def_count && MENU_ITEM_LAST != menu_def[i].selection;
	     i++)
		if ((menu_def[i].selection & selection) ==
				menu_def[i].selection) {
			if (menu_def[i].builder(data, &menu_def[i], platform))
				append_active_menu(&menu_def[i]);
		}
}

char *get_menu_item_text(unsigned int id)
{
	if (id > next_active_item)
		return NULL;

	return active_menu[id].helptext;
}

int handle_menu_item(void *data, unsigned int id)
{
	if (id > next_active_item)
		return 0;

	return (active_menu[id].handler)(data, id);
}
