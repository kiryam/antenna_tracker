#pragma once

#include "menu.h"
#include "gui.h"


#ifdef __cplusplus
extern "C" {
#endif


	Page* CreateSettingsPage(MenuItem *_menu);
	void StepperHandler();

#ifdef __cplusplus
}
#endif
