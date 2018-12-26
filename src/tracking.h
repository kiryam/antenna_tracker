#pragma once

#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

	int InitTracking();

	extern uint32_t home_dist;
	extern int16_t Elevation;
	extern int16_t Bearing;
	extern float BearingTuning;

#ifdef __cplusplus
}
#endif
