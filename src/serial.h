#pragma once

#include "FreeRTOS.h"
#include "queue.h"

extern QueueHandle_t serialRxQueue;

int serialInit();
int serialSendStr(char* str);
int serialSendBytes(uint8_t* bytes, uint32_t len);
