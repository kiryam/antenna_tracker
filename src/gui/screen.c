#include "screen.h"

#include "gfx.h"
#include "gui.h"
#include <stdbool.h>
#include "../controls.h"
#include "../telemetry.h"
#include "../gps.h"
#include "main.h"
#include "../tracking.h"

static GWidgetInit wi;
static GHandle ghContainerTelemetry, ghTelemetryRxGood, ghTelemetryRxBad, ghTelemetryPosLat, ghTelemetryPosLon;
static GHandle ghContainerGPS, ghGPSLon, ghGPSLat, ghGPSAlt, ghHomeBearing, ghGPSSats;
static GHandle ghContainerSystem, ghSystemHeapFree;
static GHandle ghContainerServo, ghServoBearing, ghServoElevation, ghServoDist;



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



void cleanScreen(){
	UIDestroyContainerWithChilds(ghContainerSystem);
	UIDestroyContainerWithChilds(ghContainerGPS);
	UIDestroyContainerWithChilds(ghContainerTelemetry);
	UIDestroyContainerWithChilds(ghContainerServo);
	//UIDestroyContainerWithChilds(ghContainerServoTuning);
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


void InfoScreenRender(){
	if( firstTimeRendered == false ){ // just create screen
		switchScreen(true);
		firstTimeRendered = true;
	}

	GEvent* pe = geventEventWait(&gl, TIME_INFINITE);

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

