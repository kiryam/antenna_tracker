#include "stm32f10x.h"
#include "main.h"
#include "FreeRTOS.h"
#include "task.h"

#include "assert.h"
#include "serial.h"
#include "cli.h"
#include "settings.h"

#define SERIAL_USART USART2
#define MAX_STRING_LEN 1024
#define MAX_QUEUE_LEN 128

QueueHandle_t serialRxQueue;


#ifdef ENABLE_SERIAL
	#include "serial.h"
	void _putchar(char character) {
		uint8_t tmp[1];
		tmp[0] = character;
		serialSendBytes(tmp, 1);
	}

	#define SERIAL_DEBUG
#endif

int serialInit() {
	TRACE_CHECKPOINT("serial init");
	serialRxQueue = xQueueCreate( MAX_QUEUE_LEN, sizeof( uint16_t ) );
	if( serialRxQueue == NULL ) {
		return 1;
	}

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

	GPIO_InitTypeDef gpio_port;
	gpio_port.GPIO_Pin   = GPIO_Pin_3;
	gpio_port.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
	gpio_port.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &gpio_port);

	GPIO_InitTypeDef gpio_port1;
	gpio_port1.GPIO_Pin   = GPIO_Pin_2;
	gpio_port1.GPIO_Speed = GPIO_Speed_50MHz;
	gpio_port1.GPIO_Mode  = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &gpio_port1);

	USART_InitTypeDef usart;
	usart.USART_BaudRate = settingsGetInt32(SERIAL_BAUD);
	usart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	usart.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	usart.USART_Parity = USART_Parity_No;
	usart.USART_StopBits = USART_StopBits_1;
	usart.USART_WordLength = USART_WordLength_8b;

	USART_Init(SERIAL_USART, &usart);
	USART_Cmd(SERIAL_USART, ENABLE);

	USART_ITConfig(SERIAL_USART, USART_IT_RXNE, ENABLE);

	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = configLIBRARY_LOWEST_INTERRUPT_PRIORITY;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	NVIC_EnableIRQ(USART2_IRQn);

	if (initCLI() != 0){
		ERROR("Init cli error");
		return 1;
	}
	TRACE_CHECKPOINT("serial done");
	return 0;
}

void USART2_IRQHandler(){
	if(USART_GetITStatus(SERIAL_USART, USART_IT_RXNE) != RESET){
		uint16_t cIn;
		BaseType_t xHigherPriorityTaskWoken;

		xHigherPriorityTaskWoken = pdFALSE;
		do {
			cIn = USART_ReceiveData (SERIAL_USART);
			xQueueSendFromISR( serialRxQueue, &cIn, &xHigherPriorityTaskWoken );
		} while( USART_GetFlagStatus(SERIAL_USART, USART_FLAG_RXNE) );

		USART_ClearITPendingBit(SERIAL_USART, USART_IT_RXNE);
		if( xHigherPriorityTaskWoken ) {
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		}
	}
}

int serialSendStr(char* str){
	int i = -1;
	taskENTER_CRITICAL(); // migrate to mutex
	while((i++)< MAX_STRING_LEN){
		if (str[i] == '\0') {
			USART_SendData(SERIAL_USART, 10);
			while(USART_GetFlagStatus(SERIAL_USART, USART_FLAG_TXE) == RESET){}
			USART_SendData(SERIAL_USART, 13);
			while(USART_GetFlagStatus(SERIAL_USART, USART_FLAG_TXE) == RESET){}
			taskEXIT_CRITICAL();
			return 0;
		};

		USART_SendData(SERIAL_USART, (uint16_t)str[i]);
		while(USART_GetFlagStatus(SERIAL_USART, USART_FLAG_TXE) == RESET){}
	}
	taskEXIT_CRITICAL();
	assert(0);
	return 1;
}

int serialSendBytes(uint8_t* bytes, uint32_t len) {
	for(unsigned int i=0; i< len; i++ ){
		USART_SendData(SERIAL_USART, bytes[i]);
		while(USART_GetFlagStatus(SERIAL_USART, USART_FLAG_TXE) == RESET){}
	}
	return 0;
}

