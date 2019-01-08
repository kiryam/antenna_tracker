#pragma once

#include "FreeRTOS.h"
#include "timers.h"
#include <stdbool.h>
#include "gfx.h"

enum activeRenderer{
	RENDER_INFO,
	RENDER_SERVO_TUNING,
	RENDER_HOME_FINDING
};

typedef struct __Page {
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
	extern GListener gl;
#ifdef __cplusplus
}
#endif
