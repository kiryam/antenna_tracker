#include <stdlib.h>
#include "main.h"
#include "gui.h"
#include "FreeRTOS.h"
#include "timers.h"
#include "stdbool.h"
#include "gfx.h"
#include "../telemetry.h"
#include "../gps.h"
#include "../tracking.h"
#include "../stepper.h"
#include "stepper_tuning.h"

#define INTCACHE_SIZE 5

static void vUIRenderTimerCallback(TimerHandle_t pxTimer);
TimerHandle_t guiTimer = NULL;
static GListener gl;
Page* currentPage;
static bool pageCreated;
int32_t intCache[INTCACHE_SIZE];


static void gwPageEvent(void *param, GEvent *pe) {
	if ( pe->type == GEVENT_TOGGLE ){
		switch( ((GEventToggle*)pe)->instance) {
		case BUTTON_ENTER:
			break;
		case BUTTON_ESC:
			//if(currentPage->Page == PAGE_SCREEN ) {
			//	switchPage(CreateServoTuningPage());
			//}else if(currentPage->Page == PAGE_SERVO_TUNING || currentPage->Page == PAGE_SETTINGS) {
			//	switchPage(CreateScreenPage());
			//}
			break;
		}
	}
}

void UIInitTask(void* pvParameters) {
    (void)pvParameters;
    gfxInit();
    font_t font = gdispOpenFont("UI2");
	gwinSetDefaultFont(font);

	GSourceHandle upHandle = ginputGetToggle(BUTTON_ESC);
	geventAttachSource(&gl, upHandle, GLISTEN_TOGGLE_ON);

	geventListenerInit(&gl);
	geventRegisterCallback(&gl, gwPageEvent, 0);

	switchPage(CreateScreenPage());

 	guiTimer = xTimerCreate( "guiTimer", 1000, pdTRUE,0, vUIRenderTimerCallback);
	if (guiTimer == NULL) {
		ERROR("Failed to create guiTimer");
	}

	if ( xTimerStart(guiTimer,  ( TickType_t ) 10) == pdFAIL ) {
		ERROR("Failed to start guiTimer");
	}

    vTaskDelete(NULL);
}

void switchPage(Page* page){
	if (page == NULL) return;

	if(currentPage != NULL){
		currentPage->Destroy(currentPage);
		vPortFree(currentPage);
	}
	currentPage = page;
	pageCreated = false;
}

void vUIRenderTimerCallback(TimerHandle_t pxTimer){
	(void) pxTimer;

	if (pageCreated == 0){
		currentPage->Create(currentPage);
		pageCreated = true;
	}
	currentPage->Render();
}

void UIDestroyContainerWithChilds(GHandle gh){
	if (gh == NULL) return;

	GHandle	child;
	while((child = gwinGetFirstChild(gh))){
		gwinDestroy(child);
	}
	gwinDestroy(gh);
}

void lat_to_char( int32_t degE7, char* buf ){
  // Extract and print negative sign
	uint8_t pos=0;
	uint8_t i=0;
	char tmp[32] ={0};

  if (degE7 < 0) {
    degE7 = -degE7;
    buf[pos++] = '-';
  }

  // Whole degrees
  int32_t deg = degE7 / 10000000L;

  itoa(deg, tmp, 10);
  while(tmp[i] != '\0' ){
	  buf[pos++] = tmp[i++];
  }
  buf[pos++] = '.';

  // Get fractional degrees
  degE7 -= deg*10000000L;

  // Print leading zeroes, if needed
  int32_t factor = 1000000L;
  while ((degE7 < factor) && (factor > 1L)){
	buf[pos++] = '0';
    factor /= 10L;
  }

  // Print fractional degrees
  //Serial.print( degE7 );
  i=0;
  memset(&tmp[0], 0, sizeof(tmp));
  itoa(degE7, tmp, 10);
  while(tmp[i] != '\0' ){
	  buf[pos++] = tmp[i++];
	}
}

void gwinSetIntCached(uint16_t cache_bank, GHandle gh, int32_t value, bool_t useAlloc){
	if (intCache[cache_bank] == value ){
		return;
	}
	intCache[cache_bank] = value;
	char tmp[32] ={0};
	itoa(value, tmp, 10);
	gwinSetText(gh, tmp, useAlloc);
}

void gwinSetDegE7Cached(uint16_t cache_bank, GHandle gh, int32_t value, bool_t useAlloc){
	if (intCache[cache_bank] == value ){
		return;
	}
	intCache[cache_bank] = value;
	char tmp[32] ={0};
	lat_to_char(value, tmp);
	gwinSetText(gh, tmp, useAlloc);
}
