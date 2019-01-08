#include "main.h"
#include "gui.h"
#include "FreeRTOS.h"
#include "timers.h"
#include "stdbool.h"
#include "gfx.h"
#include "../telemetry.h"
#include "../gps.h"
#include "../tracking.h"
#include "../controls.h"
#include "../stepper.h"

static font_t  font;
bool firstTimeRendered;
static enum activeRenderer last_renderer;
static void vUIRenderTimerCallback(TimerHandle_t pxTimer);
TimerHandle_t guiTimer = NULL;
GListener gl;

enum activeRenderer renderer;


TimerHandle_t screen_switch_timer;
uint8_t screen_sequence[SCREEN_ENUM_LEN];
uint8_t screen_i=0;
static char tmp[32] ={0};

#define SCREEN_SWITCH_TIMEOUT 3000

void UIInitTask(void* pvParameters) {
    (void)pvParameters;
    gfxInit();
    font = gdispOpenFont("UI2");
	gwinSetDefaultFont(font);

	geventListenerInit(&gl);
	gwinAttachListener(&gl);

	screen_sequence[0] = SCREEN_TELEMETRY;
	screen_sequence[1] = SCREEN_GPS;
	screen_sequence[2] = SCREEN_SYSTEM;
	screen_sequence[3] = SCREEN_SERVO;

	screen_i= 3;
	active_screen = screen_sequence[screen_i];

 	guiTimer = xTimerCreate( "guiTimer", 1000, pdTRUE,0, vUIRenderTimerCallback);
	if (guiTimer == NULL) {
		ERROR("Failed to create guiTimer");
	}

	if ( xTimerStart(guiTimer,  ( TickType_t ) 10) == pdFAIL ) {
		ERROR("Failed to start guiTimer");
	}

    vTaskDelete(NULL);
}

void vUIRenderTimerCallback(TimerHandle_t pxTimer){
	(void) pxTimer;

	if(renderer != last_renderer) {
		firstTimeRendered = false;
		last_renderer = renderer;
	}
	switch(renderer){
		case RENDER_INFO:
			InfoScreenRender();
			break;
		case RENDER_SERVO_TUNING:
			ServoTuningScreenRender();
			break;

		case RENDER_HOME_FINDING:
			HomeFinderRender();
			break;
	}
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

#define INTCACHE_SIZE 5
int32_t intCache[INTCACHE_SIZE];

void gwinSetIntCached(uint16_t cache_bank, GHandle gh, int32_t value, bool_t useAlloc){
	if (intCache[cache_bank] == value ){
		return;
	}
	intCache[cache_bank] = value;
	itoa(value, tmp, 10);
	gwinSetText(gh, tmp, useAlloc);
}

void gwinSetDegE7Cached(uint16_t cache_bank, GHandle gh, int32_t value, bool_t useAlloc){
	if (intCache[cache_bank] == value ){
		return;
	}
	intCache[cache_bank] = value;
	lat_to_char(value, tmp);
	gwinSetText(gh, tmp, useAlloc);
}
