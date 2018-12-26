#include "servo.h"
#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"

static int8_t servoCurrentAnglie;

int ServoInit() {
	TRACE_CHECKPOINT("servo init start");
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

	GPIO_InitTypeDef port;
	TIM_TimeBaseInitTypeDef timer;
	TIM_OCInitTypeDef timerPWM;

    GPIO_StructInit(&port);
    port.GPIO_Mode = GPIO_Mode_AF_PP;
    port.GPIO_Pin = GPIO_Pin_0;
    GPIO_Init(GPIOA, &port);

    TIM_TimeBaseStructInit(&timer);
    timer.TIM_Prescaler =  720-1;
    timer.TIM_Period =  2000-1;
    timer.TIM_ClockDivision = TIM_CKD_DIV1;
    timer.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &timer);

    TIM_OCStructInit(&timerPWM);
    timerPWM.TIM_Pulse = 25;
    timerPWM.TIM_OCMode = TIM_OCMode_PWM1;
    timerPWM.TIM_OutputState = TIM_OutputState_Enable;
    timerPWM.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OC1Init(TIM2, &timerPWM);

    TIM_OC1PreloadConfig(TIM2, TIM_OCPreload_Enable);
    servoCurrentAnglie = SERVO_MIN_ANGILE;
    ServoSetPos(SERVO_MIN_ANGILE);
    TIM_Cmd(TIM2, ENABLE);

    TRACE_CHECKPOINT("servo init done");
    return 0;
}

void ServoSetPos(int8_t angile) {
	if (SERVO_MIN_ANGILE < SERVO_MAX_ANGILE ){
		TIM2->CCR1 = map(angile, SERVO_MIN_ANGILE, SERVO_MAX_ANGILE, SERVO_MIN_PULSE, SERVO_MAX_PULSE);
	}else{
		TIM2->CCR1 = map(angile, SERVO_MIN_ANGILE, SERVO_MAX_ANGILE, SERVO_MAX_PULSE, SERVO_MIN_PULSE);
	}
	servoCurrentAnglie = angile;
}

void SetServoPosSmooth(int8_t angile, uint16_t degreePerSecond){
	if (angile == servoCurrentAnglie) {
		return; // do nothing
	}

	if (angile > servoCurrentAnglie){
		for(int i=servoCurrentAnglie; i<angile;i++){
			ServoSetPos(i);
			vTaskDelay(1000/degreePerSecond);
		}
	}else{
		for(int i=servoCurrentAnglie; i>angile;i--){
			ServoSetPos(i);
			vTaskDelay(1000/degreePerSecond);
		}
	}
}
