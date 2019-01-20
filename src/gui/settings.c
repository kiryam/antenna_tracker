#include <stddef.h>
#include <string.h>
#include "settings.h"
#include "../settings.h"
#include "gui.h"
#include "main.h"
#include "menu.h"


const GWidgetStyle MyCustomStyle = {
	Black,			// window background
	Black,			// focused

	// enabled color set
	{
		White,		// text
		White,		// edge
		White,		// fill
		Black		// progress - active area
	},

	// disabled color set
	{
		White,		// text
		Black,		// edge
		Black,		// fill
		White		// progress - active area
	},

	// pressed color set
	{
		White,		// text
		White,		// edge
		White,		// fill
		White		// progress - active area
	}
};
static GListener gl;
static GSourceHandle upHandle,upHandle2;
static uint16_t last_encoder_value;
static GHandle ghContainerSettings, ghList1;
//MenuItem* _activeMenu;
void (*_itemHandlers[MAX_MENU_ITEMS_COUNT])(void);

static void gwSettingsEvent(void *param, GEvent *pe){
	if ( pe->type == GEVENT_TOGGLE ){
		switch( ((GEventToggle*)pe)->instance) {
		case BUTTON_ENTER:
			_itemHandlers[gwinListGetSelected(ghList1)]();
			break;
		case BUTTON_ESC:
			//if(currentPage->Page == PAGE_SETTINGS){
			switchPage(CreateScreenPage());
			//}else {
			//	switchPage(CreateSettingsPage(NULL));
			//}
			break;
		}
	}else if( pe->type == GEVENT_DIAL) {
		switch( ((GEventDial*)pe)->instance) {
			case 0:
				if ( ((GEventDial*)pe)->value != last_encoder_value ) {
					last_encoder_value = ((GEventDial*)pe)->value;
				}
				break;
			}
	}
}

// HANDLERS
void StepperTuning(){
	switchPage(CreateStepperTuningPage());
}

void StepperOff(){
	settingsSetInt32(STEPPER_ENABLED, 0);
	INFO("Stepper off");
	switchPage(CreateSettingsPage(NULL));
}

void StepperOn(){
	settingsSetInt32(STEPPER_ENABLED, 1);
	INFO("Stepper on");
	switchPage(CreateSettingsPage(NULL));
}

void ServoTuning(){
	switchPage(CreateServoTuningPage());
}

void ServoOff(){
	settingsSetInt32(SERVO_ENABLED, 0);
	INFO("Servo off");
	switchPage(CreateSettingsPage(NULL));
}

void ServoOn(){
	settingsSetInt32(SERVO_ENABLED, 1);
	INFO("Servo on");
	switchPage(CreateSettingsPage(NULL));
}

void StepperHandler(){
	MenuItem * m = MenuCreate(3);
	if (m!= NULL){
		MenuAddItem(m, "Stepper Tuning", StepperTuning);
		MenuAddItem(m, "Stepper on", StepperOn);
		MenuAddItem(m, "Stepper off", StepperOff);
	}
	switchPage(CreateSettingsPage(m));
	INFO("Stepper settings");
}

void ServoHandler(){
	MenuItem * m = MenuCreate(3);
	if (m!= NULL){
		MenuAddItem(m, "Servo Tuning", ServoTuning);
		MenuAddItem(m, "Servo on", ServoOn);
		MenuAddItem(m, "Servo off", ServoOff);
	}
	switchPage(CreateSettingsPage(m));
	INFO("Servo settings");
}
// --------

void SettingsCreate(Page* page){
	geventListenerInit(&gl);
	gwinAttachListener(&gl);
	geventRegisterCallback(&gl, gwSettingsEvent, 0);

	upHandle = ginputGetToggle(BUTTON_ENTER);
	geventAttachSource(&gl, upHandle, GLISTEN_TOGGLE_ON);

	upHandle2 = ginputGetToggle(BUTTON_ESC);
	geventAttachSource(&gl, upHandle2, GLISTEN_TOGGLE_ON);

	GWidgetInit	wi;
	gwinWidgetClearInit(&wi);

	wi.g.show = TRUE;
	wi.g.width = 128;
	wi.g.height = 64;
	ghContainerSettings = gwinContainerCreate(0, &wi, 0);

	gwinWidgetClearInit(&wi);
	wi.customStyle = &MyCustomStyle;
	wi.g.width = 126;
	wi.g.height = 64;
	wi.text = "List Name";
	wi.g.parent = ghContainerSettings;

	ghList1 = gwinListCreate(NULL, &wi, FALSE);
	gwinListSetScroll(ghList1, scrollAuto);

	if (!gwinAttachDial(ghList1, 0, 0) ){
		ERROR("Failed to attach dial to list");
	}

	if (page->payload != NULL){
		struct menuMeta* meta = MenuGetMeta(page->payload);
		static MenuItem* ptr;
		ptr = page->payload;
		if (meta != NULL){
			for (size_t i=0; i<meta->length;i++){
				if(i>=MAX_MENU_ITEMS_COUNT) break;
				_itemHandlers[gwinListAddItem(ghList1, ptr->Title, true)] = ptr->Handler;
				ptr++;
			}
		}
	}
}

void SettingsDestroy(struct __Page* page){
	if (page->payload != NULL){
		MenuDelete(page->payload);
		page->payload = NULL;
	}

	geventDetachSourceListeners(upHandle);
	geventDetachSource(&gl, upHandle);

	geventDetachSourceListeners(upHandle2);
	geventDetachSource(&gl, upHandle2);

	vSemaphoreDelete(gl.waitqueue);

	UIDestroyContainerWithChilds(ghContainerSettings);
	ghContainerSettings = NULL;
}

void SettingsRender(){
	gwinSetVisible(ghList1, TRUE);
}

Page* CreateSettingsPage(MenuItem *_menu){
	Page* page = pvPortMalloc(sizeof(Page));
	if( page == NULL ){
		return NULL;
	}

	if (_menu == NULL){
		MenuItem * m = MenuCreate(2);
		if (m!= NULL){
			MenuAddItem(m, "Stepper", StepperHandler);
			MenuAddItem(m, "Servo", ServoHandler);
			page->payload = m;
		}
	}else{
		page->payload = _menu;
	}

	page->Page = PAGE_SETTINGS;
	page->Render = SettingsRender;
	page->Create = SettingsCreate;
	page->Destroy = SettingsDestroy;
	return page;
}
