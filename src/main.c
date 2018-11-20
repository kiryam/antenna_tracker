#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include "diag/Trace.h"

#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "gfx.h"
#include "gui.h"
#include "telemetry.h"
#include "printf.h"
#include "gps.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wreturn-type"

#ifdef ENABLE_SERIAL
	void _putchar(char character) {
		serialSendBytes(&character, 1);
	}

	#define SERIAL_DEBUG
	#include "serial.h"
	#include "cli.h"
#endif

#define ENABLE_GPS
#ifdef ENABLE_GPS
	#include "gps.h"
#endif


void DefaultTask(void * argument);

int main(int argc, char* argv[]) {
	TaskHandle_t xHandle = NULL;
	static uint8_t ucParameterToPass;
	xTaskCreate(DefaultTask, "defaultTask", configMINIMAL_STACK_SIZE, &ucParameterToPass, tskIDLE_PRIORITY, &xHandle );

	vTaskStartScheduler();
	return 0;
}


void DefaultTask(void * argument) {
	TaskHandle_t xHandle = NULL;
	static uint8_t ucParameterToPass;

	#ifdef ENABLE_SERIAL
		serialInit();
		initCLI();
		xTaskCreate(vCommandConsoleTask, "commandConsoleTask", configMINIMAL_STACK_SIZE, &ucParameterToPass, tskIDLE_PRIORITY+1, &xHandle );
	#endif

	if ( InitTelemetry() != 0 ) {
		#ifdef ENABLE_SERIAL
			ERROR("Telemetry init error");
		#endif
	} else {
		#ifdef ENABLE_SERIAL
			INFO("Telemetry init ok");
		#endif
	}

	#ifdef ENABLE_GPS
		if ( InitGPS() != 0 ){
			#ifdef ENABLE_SERIAL
				ERROR("GPS init error\n");
			#endif
		} else {
			#ifdef ENABLE_SERIAL
				INFO("GPS init done at", GPS_BAUD);
			#endif
		}
	#endif

	if (xTaskCreate(UIRenderTask, "uiRenderTask", configMINIMAL_STACK_SIZE*2, &ucParameterToPass, tskIDLE_PRIORITY, &xHandle ) != pdPASS ) {
		#ifdef SERIAL_DEBUG
			ERROR("UI render failed to start");
		#endif
	}else {
		INFO("UI started");
	}

	while (1) {
		vTaskDelay(1000);
	}

}

void vApplicationTickHook(){

}
void vApplicationStackOverflowHook( TaskHandle_t *pxTask, signed char*pcTaskName ){
	ERROR("Stack overflow: %s", pcTaskName);
}

#pragma GCC diagnostic pop
