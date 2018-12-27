#pragma once

#define ENABLE_SERIAL
#define ENABLE_GPS
#define ENABLE_TELEMTRY
#define ENABLE_STEPPER
#define ENABLE_SERVO
#define ENABLE_COMPASS
#define ENABLE_GUI
#define ENABLE_TRACKING
#define ENABLE_CONTROLS

#define COMMAND_STEPPER
#define COMMAND_TASKS


#include "stdbool.h"
#include "stdint.h"
#include "trcRecorder.h"

#define PI 3.14159265359
#define M_PI           3.14159265358979323846  /* pi */
#define MAGDEC  674 //+11° 14'

#define map(x, in_min, in_max, out_min, out_max)  (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;

#ifdef ENABLE_SERIAL
	#include "printf.h"
	#define INFO(info, ...) printf("[INFO]:  " info "\r\n", ##__VA_ARGS__)
	#define WARN(warn, ...) printf("[WARN]:  " warn "\r\n", ##__VA_ARGS__)
	#define ERROR(err, ...) printf("[ERR ]:  " err "\r\n", ##__VA_ARGS__)
#else
	#define INFO(info, ...) asm("nop");
	#define WARN(warn, ...) asm("nop");
	#define ERROR(error, ...) asm("nop");
#endif

#ifdef ENABLE_GUI
	#ifndef ENABLE_COMPASS
		#error "ENABLE_COMPASS should be defined with ENABLE_GUI";
	#endif
#endif

#define TRACE_INIT() traceChn = xTraceRegisterString("tracing")
#define TRACE_CHECKPOINT(string) vTracePrint(traceChn, string)

#ifdef ENABLE_TELEMTRY
	#define TELEMETRY_FAKE // define it to set fake telemetry values eg LON, LAT, alt; it will be updated after telemetry will be received
	#ifdef TELEMETRY_FAKE
		#define TELEMETRY_FAKE_ALT 1000*10 // 1000m

		//#define TELEMETRY_FAKE_LAT 558444992
		//#define TELEMETRY_FAKE_LON 373717152

		//#define TELEMETRY_FAKE_LAT 555066512
		//#define TELEMETRY_FAKE_LON 372232240

		// yurlovo
		#define TELEMETRY_FAKE_LAT 5589406200
		#define TELEMETRY_FAKE_LON 3725145700
	#endif
#endif


#ifdef ENABLE_GPS
	#define GPS_FAKE
	#ifdef GPS_FAKE
		//Yurlovo
		#define GPS_FAKE_LAT 558944890
		#define GPS_FAKE_LON 372504560
		#define GPS_FAKE_ALT 300*10 // 300m

		//#define GPS_FAKE_LAT 558444992
		//#define GPS_FAKE_LON 373717152
		//#define GPS_FAKE_ALT 300*10 // 300m
	#endif
#endif

#define DelayCOUNT	10

extern int16_t encoder_value;
extern bool btn1;
extern bool btn2;
extern uint16_t home_bearing;
extern void* I2C_mtx;
extern traceString traceChn;
void delayUS_DWT(uint32_t us);

