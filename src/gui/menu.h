#pragma once

#include <stdint.h>
#include <stdbool.h>


#define MAX_MENU_COUNT 2
#define MAX_MENU_ITEMS_COUNT 16

typedef struct __MenuItem {
	char Title[32];
	void (*Handler) ();
} MenuItem;

struct menuMeta{
	uint16_t max_length;
	uint16_t length;
	MenuItem* item;
};

void MenuDelete(MenuItem *m);
bool MenuAddItem(MenuItem *m, char* title, void* handler);
struct menuMeta* MenuGetMeta(MenuItem *m);
MenuItem* MenuCreate(uint16_t length);
static struct menuMeta* menuInfo[MAX_MENU_COUNT];

