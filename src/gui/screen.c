#include "main.h"
#include "screen.h"
#include "gfx.h"
#include "gui.h"
#include <stdbool.h>
#include "../telemetry.h"
#include "../gps.h"
#include "../tracking.h"

static GWidgetInit wi;
static GHandle ghContainerTelemetry, ghTelemetryRxGood, ghTelemetryRxBad, ghTelemetryPosLat, ghTelemetryPosLon;
static GHandle ghContainerGPS, ghGPSLon, ghGPSLat, ghGPSAlt, ghHomeBearing, ghGPSSats;
static GHandle ghContainerSystem, ghSystemHeapFree;
static GHandle ghContainerServo, ghServoBearing, ghServoElevation, ghServoDist;
static GListener gl;
static GSourceHandle upHandle;
void switchScreen(bool forward);
void cleanScreen();

static void gwScreenEvent(void *param, GEvent *pe) {
	if ( pe->type == GEVENT_TOGGLE ){
		switch( ((GEventToggle*)pe)->instance) {
		case 0:
			switchScreen(true);
			break;
		}
	}
}

void ScreenCreate(){
	active_screen = SCREEN_ENUM_LEN;
	switchScreen(true);
	upHandle = ginputGetToggle(0);
	geventListenerInit(&gl);
	geventAttachSource(&gl, upHandle, GLISTEN_TOGGLE_ON);
	geventRegisterCallback(&gl, gwScreenEvent, 0);
}

void ScreenDestroy(){
	cleanScreen();
	geventDetachSourceListeners(upHandle);
	geventDetachSource(&gl, upHandle);
	vSemaphoreDelete(gl.waitqueue);
}

void ScreenRender(){
	if (active_screen == SCREEN_TELEMETRY ){
		gwinSetIntCached(0, ghTelemetryRxGood, telemetry.rx_good, TRUE);
		gwinSetIntCached(1, ghTelemetryRxBad, telemetry.rx_bad, TRUE);
		gwinSetDegE7Cached(2, ghTelemetryPosLat, telemetry.lat, TRUE);
		gwinSetDegE7Cached(3, ghTelemetryPosLon, telemetry.lon, TRUE);
	} else if (active_screen == SCREEN_SYSTEM ){
		gwinSetIntCached(0, ghSystemHeapFree, xPortGetFreeHeapSize(), TRUE);
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


Page* CreateScreenPage(){
	Page* page = pvPortMalloc(sizeof(Page));
	if( page == NULL ){
		return NULL;
	}
	page->Page = PAGE_SCREEN;
	page->Render = ScreenRender;
	page->Create = ScreenCreate;
	page->Destroy = ScreenDestroy;
	return page;
}

void cleanScreen(){
	UIDestroyContainerWithChilds(ghContainerSystem);
	UIDestroyContainerWithChilds(ghContainerGPS);
	UIDestroyContainerWithChilds(ghContainerTelemetry);
	UIDestroyContainerWithChilds(ghContainerServo);
}

void switchScreen(bool forward){
	cleanScreen();
	if ( forward ) {
		active_screen++;
		if ( active_screen >= SCREEN_ENUM_LEN ){
			active_screen = 0;
		}
	}else{
		active_screen--;
		if ( active_screen >= SCREEN_ENUM_LEN ){
			active_screen = SCREEN_ENUM_LEN-1;
		}
	}

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
