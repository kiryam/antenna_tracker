#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include "diag/Trace.h"

#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "timers.h"
#include "cli.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wreturn-type"

uint16_t home_bearing = 360;
void* I2C_mtx;
volatile uint32_t micros =0;
traceString traceChn;
void vMoveTimerCallback(TimerHandle_t pxTimer);

#ifdef ENABLE_SERIAL
	#include "serial.h"
#endif

#ifdef ENABLE_GPS
	#include "gps.h"
#endif

#ifdef ENABLE_TELEMTRY
	#include "telemetry.h"
#endif

#ifdef ENABLE_COMPASS
	#include "compass.h"
#endif

#ifdef ENABLE_STEPPER
	#include "stepper.h"
#endif

#ifdef ENABLE_SERVO
	#include "servo.h"
#endif

#ifdef ENABLE_GUI
	#include "gui.h"
#endif

#ifdef ENABLE_TRACKING
	#include "tracking.h"
#endif

#ifdef ENABLE_CONTROLS
	#include "controls.h"
#endif


int main(int argc, char* argv[]) {
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

	TRACE_INIT();
	vTraceEnable(TRC_START);
	I2C_mtx = xSemaphoreCreateMutex();

	#ifdef ENABLE_SERIAL
		if ( serialInit() != 0) {
			ERROR("Failed to init serial");
		}else {
			INFO("Serial init done");
		}
	#endif

	#ifdef ENABLE_GPS
		if ( gpsInit() != 0) {
			ERROR("Failed to init gps");
		}else {
			INFO("GPS init done at %d", GPS_BAUD);
		}
	#endif

	#ifdef ENABLE_COMPASS
		if ( InitCompass() != 0) {
			ERROR("Failed to init compass");
		}else {
			INFO("Compass init done");
		}
	#endif

	#ifdef ENABLE_TELEMTRY
		if ( InitTelemetry() != 0 ) {
			ERROR("Telemetry init error");
		} else {
			INFO("Telemetry init ok");
		}
	#endif

	#ifdef ENABLE_STEPPER
		if ( StepperInit() != 0 ) {
			ERROR("Stepper init error");
		} else {
			INFO("Stepper init done");
		}
	#endif

	#ifdef ENABLE_SERVO
		if( ServoInit() != 0 ) {
			ERROR("Servo init error");
		} else {
			INFO("Servo init done");
		}
	#endif


	TaskHandle_t xHandle = NULL;
	uint8_t ucParameterToPass;

	#ifdef ENABLE_GUI
		if (xTaskCreate(UIInitTask, "uiInitTask", configMINIMAL_STACK_SIZE, &ucParameterToPass, tskIDLE_PRIORITY, &xHandle ) != pdPASS ) {
			ERROR("UI init failed to start");
		}else{
			INFO("UI init task create done");
		}
	#endif


	#ifdef ENABLE_TRACKING
		if (InitTracking() != 0 ) {
			ERROR("Tracking init error");
		} else {
			INFO("Tracking init done");
		}
	#endif

	#ifdef ENABLE_CONTROLS
		if (InitControls() != 0 ) {
			ERROR("Controls init error");
		}else {
			INFO("Controls init done");
		}
	#endif

	TimerHandle_t moveTimer = xTimerCreate( "moveTimer", 1000 ,pdFALSE,0, vMoveTimerCallback);
	if (moveTimer == NULL) {
		ERROR("Failed to create moveTimer");
		return 1;
	}
	if ( xTimerStart(moveTimer,  ( TickType_t ) 10) == pdFAIL ) {
		ERROR("Failed to start moveTimer");
		return 1;
	}

	vTaskStartScheduler();
	return 0;
}


void vMoveTimerCallback(TimerHandle_t pxTimer) {
    SetServoPosSmooth(SERVO_MAX_ANGILE,SERVO_SPEED_NORMAL);
    SetServoPosSmooth(SERVO_MIN_ANGILE,SERVO_SPEED_NORMAL);
    resetHome();
	xTimerDelete( pxTimer, 10 );
}

uint32_t getMicros(){
	return micros;
}

#ifdef __cplusplus
extern "C" {
#endif
	void vApplicationStackOverflowHook( TaskHandle_t *pxTask, signed char*pcTaskName ){
		ERROR("Stack overflow: %s", pcTaskName);
		#if defined(DEBUG)
			asm volatile ("bkpt 0");
		#endif
	}

	void vApplicationMallocFailedHook( void ) {
		ERROR("Out of memory");
		#if defined(DEBUG)
			asm volatile ("bkpt 0");
		#endif
	}

	void SysTick_Handler(){
		micros++;
		portDISABLE_INTERRUPTS();
		{
			/* Increment the RTOS tick. */
			if( xTaskIncrementTick() != pdFALSE ) {
				/* A context switch is required.  Context switching is performed in
				the PendSV interrupt.  Pend the PendSV interrupt. */
				portNVIC_INT_CTRL_REG = portNVIC_PENDSVSET_BIT;
			}
		}
		portENABLE_INTERRUPTS();
	}
#ifdef __cplusplus
}
#endif
#pragma GCC diagnostic pop
