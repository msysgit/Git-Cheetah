#ifndef CHEETAH_FINDER_MENU_H
#define CHEETAH_FINDER_MENU_H

OSStatus query_context_menu(void *me, const AEDesc *selection,
		AEDescList *menu);
OSStatus invoke_command(void *me, AEDesc *selection, SInt32 id);
void cleanup_context_menu(void *me);

#endif /* CHEETAH_FINDER_MENU_H */
