#include "main.h"
#include "gui.h"
#include "FreeRTOS.h"
#include "timers.h"
#include "stdbool.h"
#include "gfx.h"
#include "telemetry.h"
#include "gps.h"
#include "tracking.h"
#include "controls.h"
#include "stepper.h"


static GHandle ghContainerTelemetry, ghTelemetryRxGood, ghTelemetryRxBad, ghTelemetryPosLat, ghTelemetryPosLon;
static GHandle ghContainerGPS, ghGPSLon, ghGPSLat, ghGPSAlt, ghHomeBearing, ghGPSSats;
static GHandle ghContainerSystem, ghSystemHeapFree;
static GHandle ghContainerServo, ghServoBearing, ghServoElevation, ghServoDist;
static GHandle ghContainerServoTuning, ghServoBearingTuning;
static GHandle ghContainerHomeFinder, ghHomeFinderCompass;
static GWidgetInit wi;
static font_t  font;
static int16_t last_encoder_value;
static bool firstTimeRendered;
static enum activeRenderer last_renderer;
static void vUIRenderTimerCallback(TimerHandle_t pxTimer);
TimerHandle_t guiTimer = NULL;

enum activeRenderer renderer;

static enum screen_type {
	SCREEN_TELEMETRY,
	SCREEN_SYSTEM,
	SCREEN_GPS,
	SCREEN_SERVO,

	SCREEN_ENUM_LEN
} active_screen;

TimerHandle_t screen_switch_timer;
static uint8_t screen_sequence[SCREEN_ENUM_LEN];
static uint8_t screen_i=0;
static char tmp[32] ={0};

#define SCREEN_SWITCH_TIMEOUT 3000

void UIInitTask(void* pvParameters) {
    (void)pvParameters;
    gfxInit();
    font = gdispOpenFont("UI2");
	gwinSetDefaultFont(font);

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

void UITelemetryScreen(){
	gwinWidgetClearInit(&wi);
	wi.customDraw = 0;
	wi.customParam = 0;
	wi.customStyle = 0;

	wi.g.show = TRUE;
	wi.g.width = 126;
	wi.g.height = 64;
	wi.g.y = 0;
	wi.g.x = 0;
	ghContainerTelemetry = gwinContainerCreate(0, &wi, GWIN_CONTAINER_BORDER);

	wi.g.show = TRUE;
	wi.g.parent = ghContainerTelemetry;
	wi.g.width = gwinGetInnerWidth(ghContainerTelemetry);

	wi.g.y = 0;
	wi.g.x = 0;
	wi.g.height = 12;
	wi.text = "0";
	ghTelemetryRxGood = gwinLabelCreate(NULL, &wi);
	gwinLabelSetAttribute(ghTelemetryRxGood, 40, "Ok:");

	wi.g.y = 12;
	wi.g.x = 0;
	wi.g.height = 12;
	wi.text = "0";
	ghTelemetryRxBad = gwinLabelCreate(NULL, &wi);
	gwinLabelSetAttribute(ghTelemetryRxBad, 40, "Error:");


	wi.g.y = 24;
	wi.g.x = 0;
	wi.g.height = 12;
	wi.text = "0";
	ghTelemetryPosLat = gwinLabelCreate(NULL, &wi);
	gwinLabelSetAttribute(ghTelemetryPosLat, 40, "Lat:");

	wi.g.y = 36;
	wi.g.x = 0;
	wi.g.height = 12;
	wi.text = "0";
	ghTelemetryPosLon = gwinLabelCreate(NULL, &wi);
	gwinLabelSetAttribute(ghTelemetryPosLon, 40, "Lon:");
}

void UISystemInfoScreen(){
	gwinWidgetClearInit(&wi);
	wi.customDraw = 0;
	wi.customParam = 0;
	wi.customStyle = 0;

	wi.g.show = TRUE;
	wi.g.width = 126;
	wi.g.height = 64;
	wi.g.y = 0;
	wi.g.x = 0;
	ghContainerSystem = gwinContainerCreate(0, &wi, GWIN_CONTAINER_BORDER);
	wi.g.show = TRUE;
	wi.g.parent = ghContainerSystem;
	wi.g.width = gwinGetInnerWidth(ghContainerSystem);

	wi.g.show = TRUE;
	wi.g.y = 0;
	wi.g.x = 0;
	wi.g.height = 12;
	wi.text = "0";
	ghSystemHeapFree = gwinLabelCreate(NULL, &wi);
	gwinLabelSetAttribute(ghSystemHeapFree, 60, "HeapFree:");
}

void UIGPSInfoScreen() {
	gwinWidgetClearInit(&wi);
	wi.customDraw = 0;
	wi.customParam = 0;
	wi.customStyle = 0;

	wi.g.show = TRUE;
	wi.g.width = 126;
	wi.g.height = 64;
	wi.g.y = 0;
	wi.g.x = 0;
	ghContainerGPS = gwinContainerCreate(0, &wi, GWIN_CONTAINER_BORDER);
	wi.g.show = TRUE;
	wi.g.parent = ghContainerGPS;
	wi.g.width = gwinGetInnerWidth(ghContainerGPS);

	wi.g.y = 0;
	wi.g.x = 0;
	wi.g.height = 12;
	wi.text = "0";
	ghGPSSats = gwinLabelCreate(NULL, &wi);
	gwinLabelSetAttribute(ghGPSSats, 50, "Sats:");

	wi.g.y = 12;
	wi.g.x = 0;
	wi.g.height = 12;
	wi.text = "0";
	ghGPSLat = gwinLabelCreate(NULL, &wi);
	gwinLabelSetAttribute(ghGPSLat, 50, "Lat:");

	wi.g.y = 24;
	wi.g.x = 0;
	wi.g.height = 12;
	wi.text = "0";
	ghGPSLon = gwinLabelCreate(NULL, &wi);
	gwinLabelSetAttribute(ghGPSLon, 50, "Lon:");

	wi.g.y = 36;
	wi.g.x = 0;
	wi.g.height = 12;
	wi.text = "0";
	ghGPSAlt = gwinLabelCreate(NULL, &wi);
	gwinLabelSetAttribute(ghGPSAlt, 50, "Alt:");


	wi.g.y = 48;
	wi.g.x = 0;
	wi.g.height = 12;
	wi.text = "0";
	ghHomeBearing = gwinLabelCreate(NULL, &wi);
	gwinLabelSetAttribute(ghHomeBearing, 50, "Bearing:");
}

void UIServoScreen(){
	gwinWidgetClearInit(&wi);
	wi.customDraw = 0;
	wi.customParam = 0;
	wi.customStyle = 0;

	wi.g.show = TRUE;
	wi.g.width = 126;
	wi.g.height = 64;
	wi.g.y = 0;
	wi.g.x = 0;
	ghContainerServo = gwinContainerCreate(0, &wi, GWIN_CONTAINER_BORDER);
	wi.g.show = TRUE;
	wi.g.parent = ghContainerServo;
	wi.g.width = gwinGetInnerWidth(ghContainerServo);

	wi.g.y = 0;
	wi.g.x = 0;
	wi.g.height = 12;
	wi.text = "0";
	ghServoBearing = gwinLabelCreate(NULL, &wi);
	gwinLabelSetAttribute(ghServoBearing, 70, "Bearing:");

	wi.g.y = 12;
	wi.g.x = 0;
	wi.g.height = 12;
	wi.text = "0";
	ghServoElevation = gwinLabelCreate(NULL, &wi);
	gwinLabelSetAttribute(ghServoElevation, 70, "Elevation:");

	wi.g.y = 24;
	wi.g.x = 0;
	wi.g.height = 12;
	wi.text = "0";
	ghServoDist = gwinLabelCreate(NULL, &wi);
	gwinLabelSetAttribute(ghServoDist, 70, "Distantion:");
}

void UIServoTuningScreen(){
	gwinWidgetClearInit(&wi);
	wi.customDraw = 0;
	wi.customParam = 0;
	wi.customStyle = 0;

	wi.g.show = TRUE;
	wi.g.width = 126;
	wi.g.height = 64;
	wi.g.y = 0;
	wi.g.x = 0;
	ghContainerServoTuning = gwinContainerCreate(0, &wi, GWIN_CONTAINER_BORDER);
	wi.g.show = TRUE;
	wi.g.parent = ghContainerServoTuning;
	wi.g.width = gwinGetInnerWidth(ghContainerServoTuning);

	wi.g.y = 0;
	wi.g.x = 0;
	wi.g.height = 12;
	wi.text = "0";
	ghServoBearingTuning = gwinLabelCreate(NULL, &wi);
	gwinLabelSetAttribute(ghServoBearingTuning, 60, "BTuning:");
}

void UIHomeFinerScreen(){
	gwinWidgetClearInit(&wi);
	wi.customDraw = 0;
	wi.customParam = 0;
	wi.customStyle = 0;

	wi.g.show = TRUE;
	wi.g.width = 126;
	wi.g.height = 64;
	wi.g.y = 0;
	wi.g.x = 0;
	ghContainerHomeFinder = gwinContainerCreate(0, &wi, GWIN_CONTAINER_BORDER);
	wi.g.show = TRUE;
	wi.g.parent = ghContainerHomeFinder;
	wi.g.width = gwinGetInnerWidth(ghContainerHomeFinder);

	wi.g.y = 0;
	wi.g.x = 0;
	wi.g.height = 12;
	wi.text = "0";
	ghHomeFinderCompass = gwinLabelCreate(NULL, &wi);
	gwinLabelSetAttribute(ghHomeFinderCompass, 60, "Heading:");
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

void cleanScreen(){
	UIDestroyContainerWithChilds(ghContainerSystem);
	UIDestroyContainerWithChilds(ghContainerGPS);
	UIDestroyContainerWithChilds(ghContainerTelemetry);
	UIDestroyContainerWithChilds(ghContainerServo);
	UIDestroyContainerWithChilds(ghContainerServoTuning);
}

void switchScreen(bool forward){
	if ( forward ) {
		screen_i++;
		if ( screen_i >= sizeof(screen_sequence) ){
			screen_i = 0;
		}
	}else{
		screen_i--;
		if ( screen_i >= sizeof(screen_sequence) ){
			screen_i = sizeof(screen_sequence)-1;
		}
	}
	//last_encoder_value = encoder_value;

	cleanScreen();
	active_screen = screen_sequence[screen_i];
	if (active_screen == SCREEN_TELEMETRY ){
		UITelemetryScreen();
	}else if (active_screen == SCREEN_SYSTEM ){
		UISystemInfoScreen();
	}else if (active_screen == SCREEN_GPS){
		UIGPSInfoScreen();
	}else if (active_screen == SCREEN_SERVO){
		UIServoScreen();
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

void InfoScreenRender(){
	if( firstTimeRendered == false ){ // just create screen
		switchScreen(true);
		firstTimeRendered = true;
	}

	if( btn2 ){
		btn2 = false;
		renderer = RENDER_SERVO_TUNING;
		cleanScreen();
		UIServoTuningScreen();
		return;
	}

	if (btn1 ){
		switchScreen(true);
		//last_encoder_value = encoder_value;
		btn1 = false;
	}

	if (active_screen == SCREEN_TELEMETRY ){
		gwinSetIntCached(0, ghTelemetryRxGood, telemetry.rx_good, TRUE);
		gwinSetIntCached(1, ghTelemetryRxBad, telemetry.rx_bad, TRUE);
		gwinSetDegE7Cached(2, ghTelemetryPosLat, telemetry.lat, TRUE);
		gwinSetDegE7Cached(3, ghTelemetryPosLon, telemetry.lon, TRUE);
	} else if (active_screen == SCREEN_SYSTEM ){
		gwinSetIntCached(0, ghSystemHeapFree, xPortGetFreeHeapSize(), TRUE);
	} else if (active_screen == SCREEN_SYSTEM ){
		gwinSetIntCached(0,ghSystemHeapFree, xPortGetFreeHeapSize(), TRUE);
	} else if (active_screen == SCREEN_GPS ){
		gwinSetDegE7Cached(0, ghGPSLat, gps.lat, TRUE);
		gwinSetDegE7Cached(1, ghGPSLon, gps.lon, TRUE);
		gwinSetIntCached(2, ghGPSAlt, gps.alt, TRUE);
		gwinSetIntCached(3, ghGPSSats, gps.sats, TRUE);
		gwinSetIntCached(4, ghHomeBearing, home_bearing, TRUE);
	} else if (active_screen == SCREEN_SERVO) {
		gwinSetIntCached(0, ghServoBearing, Bearing, TRUE);
		gwinSetIntCached(1, ghServoElevation, Elevation, TRUE);
		gwinSetIntCached(2, ghServoDist, home_dist, TRUE);
	}
}

void ServoTuningScreenRender(){
	if( btn2 ){
		btn2 = false;
		renderer = RENDER_INFO;
		switchScreen(encoder_value);
		return;
	}

	if ( encoder_value != last_encoder_value ){
		if (encoder_value > last_encoder_value) {
			if( BearingTuning < 90 ){
				BearingTuning = BearingTuning + STEPPER_MIN_ANGILE;
			}
		} else {
			if (BearingTuning > -90 ) {
				BearingTuning = BearingTuning - STEPPER_MIN_ANGILE;
			}
		}

		last_encoder_value = encoder_value;
	}
	snprintf(tmp, 10, "%f", BearingTuning);
	gwinSetText(ghServoBearingTuning, tmp, TRUE);
}

void HomeFinderRender(){
	if( firstTimeRendered == false ){ // just create screen
		UIHomeFinerScreen();
		firstTimeRendered = true;
	}

	gwinSetIntCached(0, ghHomeFinderCompass, home_bearing, TRUE);
}
