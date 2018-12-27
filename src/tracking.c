#include "main.h"
#include "tracking.h"
#include "stdint.h"
#include "math.h"
#include "stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "telemetry.h"
#include "gps.h"
#include "minmea.h"
#include "FreeRTOS_CLI.h"
#include "timers.h"

#define sq(x) ((x)*(x))
static float lonScaleDown=0.0;               // longitude scaling

uint32_t home_dist;
int16_t Elevation;
float Bearing;
static bool tracking_log_enable;
float BearingTuning;
TimerHandle_t trackerTimer = NULL;
void vAntennaTrackingTimerCallback(TimerHandle_t pxTimer);
void calc_longitude_scaling(int32_t lat);

static BaseType_t prvTrackingLogCommand( int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString ) {
	int8_t *pcParameter1;
	BaseType_t xParameter1Length;
	pcParameter1 = FreeRTOS_CLIGetParameter ( pcCommandString, 1, &xParameter1Length);

	if (strncmp("ENABLE", (char*)pcParameter1, 6) == 0 || strncmp("enable", (char*)pcParameter1, 6) == 0) {
		tracking_log_enable = true;
		snprintf( (char*)pcWriteBuffer, xWriteBufferLen, "Tracking log enabled\r\n" );
	} else if (strncmp("DISABLE", (char*)pcParameter1, 6) == 0 || strncmp("disable", (char*)pcParameter1, 6) == 0){
		tracking_log_enable = false;
		snprintf( (char*)pcWriteBuffer, xWriteBufferLen, "Tracking log disabled\r\n" );
	} else {
		snprintf( (char*)pcWriteBuffer, xWriteBufferLen, "valid values: ENABLE or DISABLE\r\n" );
	}

	return pdFALSE;
}

static const CLI_Command_Definition_t xTasksCommand = {
	"tracking_log",
	"tracking_log <ENABLE|DISABLE>:\r\n Enable or disable track logging\r\n",
	prvTrackingLogCommand,
	1
};

int InitTracking() {
	TRACE_CHECKPOINT("tracking init start");

	if (FreeRTOS_CLIRegisterCommand( &xTasksCommand ) != pdPASS ) {
		return 1;
	}

	trackerTimer = xTimerCreate( "trackerTimer", 50 ,pdTRUE,0, vAntennaTrackingTimerCallback);
	if (trackerTimer == NULL) {
		ERROR("Failed to create trackerTimer");
		return 1;
	}

	TRACE_CHECKPOINT("tracking init done");
	return 0;
}

//static uint32_t home_alt = 2580; // 258m



#define fabs(x) x<0?-x:x

#define EARTH_RADIUS 6371000


static double deg2rad(double deg) {
    return ((deg*PI)/180);
}

/* Distance calculation using Haversine Formula.*/
double getDistanceFromLatLonInMeter(double lat1, double lon1, double lat2, double lon2) {
    double dLat = deg2rad(lat1 -lat2);
    double dLon = deg2rad(lon1 - lon2);

    /* Use Haversine Formula*/
    double a = sin(dLat/2) * sin(dLat/2) +
        cos(deg2rad(lat1)) * cos(deg2rad(lat2)) *
        sin(dLon/2) * sin(dLon/2);
    double c =2 * atan2(sqrt(a), sqrt(1-a));
    double d = EARTH_RADIUS * c;

    return d;
}


float calc_bearing(int32_t lon1, int32_t lat1, int32_t lon2, int32_t lat2) {
    float dLat = (lat2 - lat1);
    float dLon = (float)(lon2 - lon1) * lonScaleDown;
    home_dist = sqrt(sq(fabs(dLat)) + sq(fabs(dLon))) * 1.113195; // home dist in cm.
    float b =  -90 + (atan2(dLat, -dLon) * 57.295775);
    if(b < 0) b += 360;
    return b;
}

int16_t calc_elevation(int32_t alt) {
    float at = atan2(alt, home_dist);
    at = at * 57.2957795;
    int16_t e = (int16_t)round(at);
    return e;
}


void vAntennaTrackingTimerCallback(TimerHandle_t pxTimer) {
	(void) pxTimer;

	if( /*gps.fix == MINMEA_GPGSA_FIX_2D &&*/  telemetry.status == TELEMETRY_OK ) {
		calc_longitude_scaling(gps.lon);
		Bearing = calc_bearing(gps.lon,gps.lat,telemetry.current_messages.gps_raw_int.lon,telemetry.current_messages.gps_raw_int.lat);
		Elevation = calc_elevation(telemetry.current_messages.gps_raw_int.alt - gps.alt);
		//if(bearing >= home_bearing){
		//	bearing -= home_bearing;
		//} else {
		//	bearing += 360 - home_bearing;
		//}

		if (tracking_log_enable ) {
			INFO("Track to Bearing: %d Elevation: %d (home dist: %d)", Bearing, Elevation, home_dist);
		}
	}
}


void calc_longitude_scaling(int32_t lat) {
    float rads       = (abs((float)lat) / 10000000.0) * 0.0174532925;
    lonScaleDown = cos(rads);
}
