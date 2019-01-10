#pragma once

#include "FreeRTOS.h"
#include "timers.h"
#include <stdbool.h>
#include "gfx.h"

enum PageEnum{
	PAGE_SCREEN,
	PAGE_SERVO_TUNING,
	PAGE_HOME_FINDING
};

typedef struct __Page {
	enum PageEnum Page;
	void (*Create)   ();
	void (*Render)   ();
	void (*Destroy)   ();
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
