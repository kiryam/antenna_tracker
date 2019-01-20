#include "gfx.h"

#if GFX_USE_GINPUT && GINPUT_NEED_DIAL

#include "src/ginput/ginput_driver_dial.h"

static uint16_t encoder_value;

void ginput_lld_dial_init(void) {
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

	TIM_TimeBaseInitTypeDef timer3;
	TIM_TimeBaseStructInit(&timer3);
	timer3.TIM_Prescaler = 0;
	timer3.TIM_Period = 1;
	timer3.TIM_CounterMode = TIM_CounterMode_Down | TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM3, &timer3);
	TIM_EncoderInterfaceConfig(TIM3, TIM_EncoderMode_TI12, TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);
	TIM_ClearITPendingBit(TIM3,  TIM_IT_Update);
	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);

	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = configLIBRARY_LOWEST_INTERRUPT_PRIORITY-1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	NVIC_EnableIRQ(TIM3_IRQn);
	TIM_Cmd(TIM3, ENABLE);

}

void ginput_lld_dial_poll(DialCallbackFn fn) {
	fn(0, encoder_value);
}


#ifdef __cplusplus
extern "C" {
#endif
	void TIM3_IRQHandler(){
		 if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET) {
			 if  (TIM_GetCounter(TIM3) ) {
				 //direction = 0;
				 encoder_value--;
			 }else{
				 //direction = 1;
				 encoder_value++;
			 }
			 TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
		 }
	}
#ifdef __cplusplus
}
#endif

#endif /* GFX_USE_GINPUT && GINPUT_NEED_DIAL */
