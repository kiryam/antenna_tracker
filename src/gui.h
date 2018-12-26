#pragma once

#include "FreeRTOS.h"
#include "timers.h"

#ifdef __cplusplus
extern "C" {
#endif

	enum activeRenderer{
		RENDER_INFO,
		RENDER_SERVO_TUNING,
		RENDER_HOME_FINDING
	};

	void UIInitTask(void* pvParameters);
	void UIRenderTask(void * argunment);
	void switchScreen(bool formward);

	extern enum activeRenderer renderer;
	extern TimerHandle_t guiTimer;

#ifdef __cplusplus
}
#endif
