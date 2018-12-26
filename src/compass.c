#include "HMC5883L.h"
#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "math.h"

//void CompassReadTask(void *pvParameters);
static float declinationAngle;

int InitCompass() {
	TRACE_CHECKPOINT("compass init");
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_I2C1, ENABLE);

	HMC5883L_I2C_Init();
	//HMC5883L_Initialize();

	// write MODE register

	HMC5883L_SetMode(HMC5883L_MODE_CONTINUOUS);
	// // 10k continious mode
	//    HMC5883L_I2C_ByteWrite(HMC5883L_DEFAULT_ADDRESS, &buf, HMC5883L_MODE_REG);
	//    HMC5883L_I2C_ByteWrite(HMC5883L_DEFAULT_ADDRESS, &buf, HMC5883L_CONF_REGB);
//	vTaskDelay(1000);

	if ( !HMC5883L_TestConnection() ) {
		return 1;
	}

	//if ( xTaskCreate(CompassReadTask, "compassReadTask", configMINIMAL_STACK_SIZE, &ucParameterToPass, tskIDLE_PRIORITY, &xHandle ) != pdTRUE ){
	//	return 1;
	//}
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
