#include "main.h"
#include "telemetry.h"
#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "queue.h"

#include "common/mavlink.h"
#include "LightTelemetry.h"

//#define DEBUG_MAVLINK
#define TELEMETRY_USART USART1
#define TELEMETRY_QUEUE_LEN 512

QueueHandle_t telemetryRxQueue;
static mavlink_status_t lastStatus;
static mavlink_message_t message;
static uint16_t cRxedChar;
static uint8_t msgReceived = false;
static TimerHandle_t telemetryTimer = NULL;

void vTelemetryTimerCallback(TimerHandle_t pxTimer);

enum TelemetryType {
	TELEMETRY_MAVLINK,
	#ifdef PROTOCOL_LIGHTTELEMETRY
		TELEMETRY_LTM
	#endif
};

static enum TelemetryType telemetryType = TELEMETRY_MAVLINK;

TelemetryStats telemetry;

int InitTelemetry(){
	TRACE_CHECKPOINT("telemetry init");
	telemetry.status = TELEMETRY_UNKNOWN;
	telemetryType = TELEMETRY_LTM;
	#ifdef TELEMETRY_FAKE
		telemetry.alt = TELEMETRY_FAKE_ALT;
		telemetry.lat = TELEMETRY_FAKE_LAT;
		telemetry.lon = TELEMETRY_FAKE_LON;
		telemetry.status = TELEMETRY_OK;
		WARN("Telemetry fake parameters set");
	#endif

	telemetryRxQueue = xQueueCreate( TELEMETRY_QUEUE_LEN, sizeof( uint16_t ) );
	if( telemetryRxQueue == NULL ) {
		return 1;
	}

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1, ENABLE);
	GPIO_InitTypeDef gpio_port;
	gpio_port.GPIO_Pin   = GPIO_Pin_10;
	gpio_port.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
	gpio_port.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &gpio_port);

	GPIO_InitTypeDef gpio_port1;
	gpio_port1.GPIO_Pin   = GPIO_Pin_9;
	gpio_port1.GPIO_Speed = GPIO_Speed_50MHz;
	gpio_port1.GPIO_Mode  = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &gpio_port1);

	USART_InitTypeDef usart;
	usart.USART_BaudRate = 57600;
	usart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	usart.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	usart.USART_Parity = USART_Parity_No;
	usart.USART_StopBits = USART_StopBits_1;
	usart.USART_WordLength = USART_WordLength_8b;

	USART_Init(TELEMETRY_USART, &usart);
	USART_Cmd(TELEMETRY_USART, ENABLE);

	USART_ITConfig(TELEMETRY_USART, USART_IT_RXNE, ENABLE);

	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = configLIBRARY_LOWEST_INTERRUPT_PRIORITY;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	NVIC_EnableIRQ(USART1_IRQn);

	telemetryTimer = xTimerCreate( "telemetryTimer", ( 500 / portTICK_PERIOD_MS),pdTRUE,0, vTelemetryTimerCallback);
	if (telemetryTimer == NULL) {
		ERROR("Failed to create telemetryTimer");
		return 1;
	}
	if ( xTimerStart(telemetryTimer,  ( TickType_t ) 100) == pdFAIL ) {
		ERROR("Failed to start telemetryTimer");
		return 1;
	}

	TRACE_CHECKPOINT("telemetry done");
	return 0;
}


void USART1_IRQHandler(){
	if(USART_GetITStatus(TELEMETRY_USART, USART_IT_RXNE) != RESET) {
		USART_ClearITPendingBit(USART1, USART_IT_RXNE);

		uint16_t cIn;
		BaseType_t xHigherPriorityTaskWoken;

		//xHigherPriorityTaskWoken = pdFALSE;
		do {
			cIn = USART_ReceiveData (TELEMETRY_USART);
			xQueueSendFromISR( telemetryRxQueue, &cIn, &xHigherPriorityTaskWoken );
		} while( USART_GetITStatus(TELEMETRY_USART, USART_IT_RXNE) );


		USART_ClearITPendingBit(TELEMETRY_USART, USART_IT_RXNE);
		//if( xHigherPriorityTaskWoken ) {
		//	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		//}
	}
}

static mavlink_status_t status;

void vTelemetryTimerCallback(TimerHandle_t pxTimer) {
	(void)pxTimer;

	if (telemetryType == TELEMETRY_MAVLINK) {
		while( xQueueReceive( telemetryRxQueue, &( cRxedChar ), ( TickType_t ) 1 ) ) {
			msgReceived = mavlink_parse_char(MAVLINK_COMM_0, cRxedChar, &message, &status);

			if ( (lastStatus.packet_rx_drop_count != status.packet_rx_drop_count) ) {
				telemetry.rx_bad++;
			}
			lastStatus = status;

			if(msgReceived) {
				telemetry.rx_good++;
				//telemetry.current_messages.compid = message.compid;
				//telemetry.current_messages.sysid = message.sysid;

				switch (message.msgid) {
					case MAVLINK_MSG_ID_GPS_RAW_INT:
					{
						mavlink_gps_raw_int_t raw_int;
						mavlink_msg_gps_raw_int_decode(&message, &raw_int);
						telemetry.lat = raw_int.lat;
						telemetry.lon = raw_int.lon;
						telemetry.alt = raw_int.alt;
						telemetry.status = TELEMETRY_OK;
						#ifdef DEBUG_MAVLINK
							INFO("MAVLINK_MSG_ID_GPS_RAW_INT %d, %d\n", telemetry.current_messages.gps_raw_int.lon, telemetry.current_messages.gps_raw_int.lat);
						#endif

						break;
					}
					default: {
						// printf("Warning, did not handle message id %i\n",message.msgid);
						break;
					}
				}
			}
		}
	}
#ifdef PROTOCOL_LIGHTTELEMETRY
	else if (telemetryType == TELEMETRY_LTM) {
		ltm_read();
	}
#endif
}
