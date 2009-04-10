#ifndef CHEETAHMENU_H
#define CHEETAHMENU_H

void build_cheetah_menu(struct git_data *data, void *platform_data);

/*
 * Prototypes of functions that must be provided by the client
 */
void reset_platform(void *platform);
BOOL build_separator(struct git_data *data, const struct menu_item *item,
		     void *platform);
BOOL build_item(struct git_data *data, const struct menu_item *item,
		void *platform);

void *start_submenu(struct git_data *, const struct menu_item *item,
		    void *platform);
void end_submenu(void *parent, void *submenu);

/*
 * Cheetah-specific flags and functions
 *
 * Generally, nobody is interested in these flags & functions, but
 * they may be useful for unit testing.
 *
 * 2 bits to indicate each choise are required because
 * menu_item.selection is used as a mask and there are
 * actually three choices:
 * - selected;
 * - not selected;
 * - irrelevant
 */
#define MENU_ITEM_FILE    (1 << 0)
#define MENU_ITEM_DIR     (1 << 1)
#define MENU_ITEM_NOREPO  (1 << 2)
#define MENU_ITEM_REPO    (1 << 3)
#define MENU_ITEM_TRACK   (1 << 4)
#define MENU_ITEM_NOTRACK (1 << 5)

UINT cheetah_menu_mask(struct git_data *this_);

#endif /* CHEETAHMENU_H */
