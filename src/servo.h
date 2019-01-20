#pragma once


// 2000 - 20ms
// For servo  ES08MAII 90 degree
// standard servo 1ms - 1.9ms
// eg MIN 100 MAX 190

// For servo MG995 180degree
// 1ms - minimal - 4 maximal


#define SERVO_90
//#define SERVO_180

#define SERVO_STEP_ANGILE 1

#ifdef SERVO_90
	#define SERVO_MAX_PULSE 190
	#define SERVO_MIN_PULSE 80
	#define SERVO_MIN_ANGILE 0.0
	#define SERVO_MAX_ANGILE 90.0
	#define SERVO_FLIP 1
#endif

#ifdef SERVO_180
	#define SERVO_MAX_PULSE 390
	#define SERVO_MIN_PULSE 100
	#define SERVO_MIN_ANGILE 180
	#define SERVO_MAX_ANGILE 0
#endif


// degree per second
#define SERVO_SPEED_SLOW 15
#define SERVO_SPEED_NORMAL 90
#define SERVO_SPEED_FAST 180


#ifdef __cplusplus
extern "C" {
#endif
#include "stdint.h"
	int ServoInit();
	void ServoSetPos(float angile);
	void SetServoPosSmooth(float angile, uint16_t degreePerSecond);

#ifdef __cplusplus
}
#endif

extern int8_t servoTargetAngile;
