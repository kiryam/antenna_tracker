#include "HMC5883L.h"
#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "math.h"

static float declinationAngle;

int InitCompass() {
	TRACE_CHECKPOINT("compass init");
	HMC5883L_I2C_Init();
	HMC5883L_SetMode(HMC5883L_MODE_CONTINUOUS);
	if ( !HMC5883L_TestConnection() ) {
		return 1;
	}

	TRACE_CHECKPOINT("compass done");
	return 0;
}

uint16_t GetHomeBearing(){
	if( xSemaphoreTake( I2C_mtx, ( TickType_t ) 1000 ) == pdTRUE ) {
		int16_t heading_xyz[3];

		taskENTER_CRITICAL();
		HMC5883L_GetHeading(heading_xyz);
		taskEXIT_CRITICAL();

		float heading;
		heading = atan2(heading_xyz[1]*0.92, heading_xyz[0]*0.92);
		declinationAngle = MAGDEC / 1000;
		heading += declinationAngle;

		// Correct for when signs are reversed.
		if (heading < 0)    heading += 2*PI;

		// Check for wrap due to addition of declination.
		if (heading > 2*PI) heading -= 2*PI;

		// Convert radians to degrees for readability.
		home_bearing = (int)round(heading * 180/M_PI);
		//INFO("Home bearing %d", home_bearing);
		xSemaphoreGive( I2C_mtx);
	}else{
		return 0;
	}

	return home_bearing;
}

//void CompassReadTask(void *pvParameters){
//	(void) pvParameters;
//	for(;;) {
//	}
	// Once you have your heading, you must then add your ‘Declination Angle’, which is the ‘Error’ of the magnetic field in your location.
	// Find yours here: http://www.magnetic-declination.com/
//}
