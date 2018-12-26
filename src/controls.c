#include "controls.h"
#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "task.h"
#include "stdbool.h"
#include "main.h"
#include "timers.h"
#include "gui.h"

bool btn1;
bool btn2;
int16_t encoder_value;
uint8_t direction;

int InitControls(){
	TRACE_CHECKPOINT("controls init");
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

	NVIC_InitTypeDef NVIC_InitStructure;
	EXTI_InitTypeDef EXTI_InitStruct;

	GPIO_InitTypeDef gpio_port;
	GPIO_StructInit(&gpio_port);
	gpio_port.GPIO_Pin   = GPIO_Pin_4;
	gpio_port.GPIO_Mode  = GPIO_Mode_IPU;
	gpio_port.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &gpio_port);

	EXTI_StructInit(&EXTI_InitStruct);
	EXTI_InitStruct.EXTI_Line = EXTI_Line4;
	EXTI_InitStruct.EXTI_LineCmd = ENABLE;
	EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
	EXTI_Init(&EXTI_InitStruct);

	NVIC_InitStructure.NVIC_IRQChannel = EXTI4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = configLIBRARY_LOWEST_INTERRUPT_PRIORITY-1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	GPIO_StructInit(&gpio_port);
	gpio_port.GPIO_Pin   = GPIO_Pin_1;
	gpio_port.GPIO_Mode  = GPIO_Mode_IPU;
	gpio_port.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &gpio_port);

	EXTI_StructInit(&EXTI_InitStruct);
	EXTI_InitStruct.EXTI_Line = EXTI_Line1;
	EXTI_InitStruct.EXTI_LineCmd = ENABLE;
	EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
	EXTI_Init(&EXTI_InitStruct);

	NVIC_InitStructure.NVIC_IRQChannel = EXTI1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = configLIBRARY_LOWEST_INTERRUPT_PRIORITY-1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	TIM_TimeBaseInitTypeDef timer3;
	TIM_TimeBaseStructInit(&timer3);
	timer3.TIM_Prescaler = 0;
	timer3.TIM_Period = 1;
	timer3.TIM_CounterMode = TIM_CounterMode_Down | TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM3, &timer3);
	TIM_EncoderInterfaceConfig(TIM3, TIM_EncoderMode_TI1, TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);
	TIM_ClearITPendingBit(TIM3,  TIM_IT_Update);
	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);

	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = configLIBRARY_LOWEST_INTERRUPT_PRIORITY-1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	NVIC_EnableIRQ(TIM3_IRQn);
	TIM_Cmd(TIM3, ENABLE);
	TRACE_CHECKPOINT("controls init done");

	return 0;
}

#ifdef __cplusplus
extern "C" {
#endif
	void TIM3_IRQHandler(){
		 if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET) {
			 if  (TIM_GetCounter(TIM3) ) {
				 direction = 0;
				 encoder_value--;
			 }else{
				 direction = 1;
				 encoder_value++;
			 }
			 TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
		 }
	}

	void EXTI4_IRQHandler(){
		if (EXTI_GetITStatus(EXTI_Line4) != RESET) {
			if ( GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_4) == Bit_RESET ) {
				btn2 = true;
			}else {
				btn2 = false;
			}
			EXTI_ClearITPendingBit(EXTI_Line4);
		}
	}

	void EXTI1_IRQHandler(){
		 if (EXTI_GetITStatus(EXTI_Line1) != RESET) {
			if ( GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_1) == Bit_RESET ) {
				btn1 = true;
			}else {
				btn1 = false;
			}
			//BaseType_t xHigherPriorityTaskWoken = pdFALSE;
			//xTimerResetFromISR( guiTimer, &xHigherPriorityTaskWoken ) ;
			//if( xHigherPriorityTaskWoken ) {
			//	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
			//}
			EXTI_ClearITPendingBit(EXTI_Line1);
		}
	}
#ifdef __cplusplus
}
#endif

