#pragma once

#include "FreeRTOS.h"
#include "timers.h"
#include <stdbool.h>
#include "gfx.h"

#ifdef __cplusplus
extern "C" {
#endif

	enum activeRenderer{
		RENDER_INFO,
		RENDER_SERVO_TUNING,
		RENDER_HOME_FINDING
	};


	static enum screen_type {
		SCREEN_TELEMETRY,
		SCREEN_SYSTEM,
		SCREEN_GPS,
		SCREEN_SERVO,

		SCREEN_ENUM_LEN
	} active_screen;

	void UIInitTask(void* pvParameters);
	void UIRenderTask(void * argunment);
	void switchScreen(bool formward);

	extern enum activeRenderer renderer;
	extern TimerHandle_t guiTimer;
	extern bool firstTimeRendered;
	extern uint8_t screen_i;

	extern uint8_t screen_sequence[SCREEN_ENUM_LEN];
	extern GListener gl;

#ifdef __cplusplus
}
#endif
