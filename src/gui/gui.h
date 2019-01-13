#pragma once

#include "FreeRTOS.h"
#include "timers.h"
#include <stdbool.h>
#include "gfx.h"

enum PageEnum{
	PAGE_SCREEN,
	PAGE_SERVO_TUNING,
	PAGE_HOME_FINDING,
	PAGE_SETTINGS
};


enum ButtonEnum{
	BUTTON_ENTER,
	BUTTON_ESC
};

typedef struct __Page {
	enum PageEnum Page;
	void (*Create)   (struct __Page* page);
	void (*Render)  ();
	void (*Destroy)  (struct __Page* page);
	void* payload;
} Page;

#ifdef __cplusplus
extern "C" {
#endif

	void UIInitTask(void* pvParameters);
	void UIRenderTask(void * argunment);
	void switchPage(Page* page);
	void UIDestroyContainerWithChilds(GHandle gh);
	extern TimerHandle_t guiTimer;
#ifdef __cplusplus
}
#endif
