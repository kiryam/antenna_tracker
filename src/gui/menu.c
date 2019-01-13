#include <stdbool.h>
#include <stddef.h>
#include "menu.h"

MenuItem* MenuCreate(uint16_t length){
	for (int i =0; i<MAX_MENU_ITEMS_COUNT; i++){
		if(menuInfo[i] == NULL){
			MenuItem* m = pvPortMalloc(sizeof(MenuItem)*length);
			memset(m, 0, sizeof(MenuItem));

			struct menuMeta* meta = pvPortMalloc(sizeof(struct menuMeta));
			memset(meta, 0, sizeof(struct menuMeta));
			meta->max_length = length;
			meta->item = m;
			menuInfo[i] = meta;
			return m;
		}
	}
	return false;
}

struct menuMeta* MenuGetMeta(MenuItem *m){
	for (int i =0; i<MAX_MENU_COUNT; i++){
		struct menuMeta* _m = menuInfo[i];
		if (_m != NULL && _m->item == m) {
			return _m;
		}
	}

	return NULL;
}

bool MenuAddItem(MenuItem *m, char* title, void* handler){
	struct menuMeta* meta = MenuGetMeta(m);
	if(meta == NULL) return false;
	if(meta->length >= meta->max_length) return false;

	strcpy(&meta->item[meta->length].Title, title);
	meta->item[meta->length].Handler = handler;
	meta->length++;
	return true;
}

void MenuDelete(MenuItem *m){
	if(m == NULL) return;
	for (int i =0; i<MAX_MENU_COUNT; i++){
		struct menuMeta* _m = menuInfo[i];
		if (_m != NULL && _m->item == m) {
			vPortFree(_m);
			menuInfo[i] = NULL;
			break;
		}
	}

	vPortFree(m);
}
