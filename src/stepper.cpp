#include "BasicStepperDriver.h"
#include "stm32f10x.h"
#include "FreeRTOS.h"

#include "task.h"
#include "compass.h"
#include "stdlib.h"
#include "tracking.h"
#include "gui/gui.h"
#include "timers.h"
#include "stepper.h"
#include "settings.h"

#define STEPPER_DIR GPIO_Pin_11
#define STEPPER_STEP GPIO_Pin_12
#define STEPPER_ENABLE GPIO_Pin_7

void pointTo(float angile);
void vStepperTimerCallback(TimerHandle_t pxTimer);
static TimerHandle_t stepperTimer = NULL;

static float currentAngile = 0;
BasicStepperDriver stepper(MOTOR_STEPS, GPIOA, STEPPER_DIR, GPIOA, STEPPER_STEP, GPIOB, STEPPER_ENABLE);


#ifdef COMMAND_STEPPER
#ifdef __cplusplus
extern "C" {
#endif
	#include "FreeRTOS_CLI.h"
#ifdef __cplusplus
}
#endif

static BaseType_t prvStepperAngileCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString ) {
	const char *pcParameter1;
	BaseType_t xParameter1Length;
	pcParameter1 = FreeRTOS_CLIGetParameter ( pcCommandString, 1, &xParameter1Length);
	snprintf( (char*)pcWriteBuffer, xWriteBufferLen, "Moving to angile %d\r\n", atoi(pcParameter1) );
	pointTo(atoi(pcParameter1));
	return pdFALSE;
}

static const CLI_Command_Definition_t xTasksCommand = {
	"stepper_move_angile",
	"stepper_move_angile <ANGILE>:\r\n Point to angile [-360 - +360]\r\n",
	prvStepperAngileCommand,
	1
};

#endif

int StepperInit() {
	#ifdef COMMAND_STEPPER
		FreeRTOS_CLIRegisterCommand( &xTasksCommand );
	#endif

	TRACE_CHECKPOINT("stepper init start");
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

	GPIO_InitTypeDef gpio_port;
	GPIO_StructInit(&gpio_port);
	gpio_port.GPIO_Pin   = STEPPER_DIR | STEPPER_STEP;
	gpio_port.GPIO_Mode  = GPIO_Mode_Out_PP;
	gpio_port.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &gpio_port);

	GPIO_StructInit(&gpio_port);
	gpio_port.GPIO_Pin   = STEPPER_ENABLE;
	gpio_port.GPIO_Mode  = GPIO_Mode_Out_PP;
	gpio_port.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &gpio_port);

	stepper.setSpeedProfile(stepper.LINEAR_SPEED, 8, 8);
 	stepper.begin(60, 16);


 	stepperTimer = xTimerCreate( "stepperTimer", 100, pdTRUE,0, vStepperTimerCallback);
	if (stepperTimer == NULL) {
		ERROR("Failed to create stepperTimer");
		return 1;
	}

	TRACE_CHECKPOINT("stepper init done");
	return 0;
}

int resetHome(){
	GetHomeBearing();
	//renderer = RENDER_HOME_FINDING;

	if ( xTimerStart(stepperTimer,  ( TickType_t ) 10) == pdFAIL ) {
		ERROR("Failed to start stepperTimer");
		return 1;
	}

	// move until 0 degree;

	currentAngile = 0;
	//renderer = RENDER_INFO;

	return 0;
}

void vStepperTimerCallback(TimerHandle_t pxTimer){
	( void ) pxTimer;
	pointTo(Bearing+BearingTuning);
}

void pointTo(float angile){
	if (settingsGetInt32(STEPPER_ENABLED) == 0) return;
	TRACE_CHECKPOINT("POINT TO START");
	float diff = angile-currentAngile;

	if ( diff  == 0 ) {  // its only 1 degree step. TODO make float calculations
		return;
	}
	float dir = -1;
	if ( diff > 180) { // rotate counter wise
		dir = 1;
		diff = (float)360-diff;
	}
	//taskENTER_CRITICAL();
	stepper.rotate(diff * dir);
	//taskEXIT_CRITICAL();

	currentAngile = angile;
	INFO("Stepper rotate: %f, current angile: %f", -1*diff*dir, currentAngile);
	TRACE_CHECKPOINT("POINT TO DONE");
}
