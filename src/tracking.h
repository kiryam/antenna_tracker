#pragma once

#include "stdint.h"
#include "FreeRTOS.h"
#include "timers.h"

#ifdef __cplusplus
extern "C" {
#endif

	int InitTracking();

	extern uint32_t home_dist;
	extern float Elevation;
	extern float ElevationTuning;
	extern float Bearing;
	extern float BearingTuning;
	extern TimerHandle_t trackerTimer;

#ifdef __cplusplus
}
#endif
