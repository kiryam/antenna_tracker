#pragma once

#include "FreeRTOS.h"
#include "queue.h"
extern QueueHandle_t serialRxQueue;

#ifdef __cplusplus
extern "C" {
#endif

	int serialInit();
	int serialSendStr(char* str);
	int serialSendBytes(uint8_t* bytes, uint32_t len);

#ifdef __cplusplus
}
#endif
