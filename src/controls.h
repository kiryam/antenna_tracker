#pragma once

#include "stdbool.h"
#include "stdint.h"

#define butDEBOUNCE_DELAY 100

#ifdef __cplusplus
extern "C" {
	#endif

	int InitControls();

#ifdef __cplusplus
}
#endif

extern uint8_t direction;
extern int16_t encoder_value;
extern bool btn1;
extern bool btn2;
