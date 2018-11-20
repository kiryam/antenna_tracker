#include "main.h"
#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "FreeRTOS_CLI.h"
#include "queue.h"
#include "stdbool.h"
#include "minmea.h"
#include "serial.h"
#include "string.h"
#include "gps.h"

#define GPS_USART USART3
#define GPS_USART_IRQ USART3_IRQn
#define GPS_QUEUE_LEN MINMEA_MAX_LENGTH


void vGPSReadTask(void * argument);

static QueueHandle_t gpsRxQueue;
static char line[MINMEA_MAX_LENGTH];
static uint16_t line_i;
static uint16_t cRxedChar;
static struct minmea_sentence_rmc rmc_frame;
static struct minmea_sentence_gsv frame_gsv;
static struct minmea_sentence_gga frame_gga;
static bool gps_log_enable;

#define CLEAR_LINE() memset(&line[0], 0, MINMEA_MAX_LENGTH); line_i = 0

static BaseType_t prvGPSLogCommand( int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString ) {
	int8_t *pcParameter1;
	BaseType_t xParameter1Length;
	pcParameter1 = FreeRTOS_CLIGetParameter ( pcCommandString, 1, &xParameter1Length);

	if (strncmp("ENABLE", (char*)pcParameter1, 6) == 0 || strncmp("enable", (char*)pcParameter1, 6) == 0) {
		gps_log_enable = true;
		snprintf( (char*)pcWriteBuffer, xWriteBufferLen, "GPS log enabled\r\n" );
	} else if (strncmp("DISABLE", (char*)pcParameter1, 6) == 0 || strncmp("disable", (char*)pcParameter1, 6) == 0){
		gps_log_enable = false;
		snprintf( (char*)pcWriteBuffer, xWriteBufferLen, "GPS log disabled\r\n" );
	} else {
		snprintf( (char*)pcWriteBuffer, xWriteBufferLen, "valid values: ENABLE or DISABLE\r\n" );
	}

	return pdFALSE;
}

static const CLI_Command_Definition_t xTasksCommand = {
	"gps_log",
	"\r\gps_log <ENABLE|DISABLE>:\r\n Enable or disable gps output logging\r\n",
	prvGPSLogCommand,
	1
};

int InitGPS() {
	gpsRxQueue = xQueueCreate( GPS_QUEUE_LEN, sizeof( uint16_t ) );
	if( gpsRxQueue == NULL ) {
		return 1;
	}

	TaskHandle_t xHandle = NULL;
	static uint8_t ucParameterToPass;
	if ( xTaskCreate(vGPSReadTask, "gpsReadTask", configMINIMAL_STACK_SIZE, &ucParameterToPass, tskIDLE_PRIORITY, &xHandle ) != pdPASS) {
		return 1;
	}

	if (FreeRTOS_CLIRegisterCommand( &xTasksCommand ) != pdPASS ) {
		return 1;
	}

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

	GPIO_InitTypeDef gpio_port;
	gpio_port.GPIO_Pin   = GPIO_Pin_11;
	gpio_port.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
	gpio_port.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &gpio_port);

	GPIO_InitTypeDef gpio_port1;
	gpio_port1.GPIO_Pin   = GPIO_Pin_10;
	gpio_port1.GPIO_Speed = GPIO_Speed_50MHz;
	gpio_port1.GPIO_Mode  = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOB, &gpio_port1);

	USART_InitTypeDef usart;
	usart.USART_BaudRate = GPS_BAUD;
	usart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	usart.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	usart.USART_Parity = USART_Parity_No;
	usart.USART_StopBits = USART_StopBits_1;
	usart.USART_WordLength = USART_WordLength_8b;

	USART_Init(GPS_USART, &usart);
	USART_Cmd(GPS_USART, ENABLE);

	USART_ITConfig(GPS_USART, USART_IT_RXNE, ENABLE);

	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = GPS_USART_IRQ;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	NVIC_EnableIRQ(GPS_USART_IRQ);
	return 0;
}

void USART3_IRQHandler(){
	uint16_t cIn;
	BaseType_t xHigherPriorityTaskWoken;

	xHigherPriorityTaskWoken = pdFALSE;
	do {
		cIn = USART_ReceiveData (GPS_USART);
		USART_ClearITPendingBit(GPS_USART, USART_IT_RXNE);
		xQueueSendFromISR( gpsRxQueue, &cIn, &xHigherPriorityTaskWoken );
	} while( USART_GetITStatus(GPS_USART, USART_IT_RXNE) );

	if( xHigherPriorityTaskWoken ) {
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}
}

void vGPSReadTask(void * argument) {
	(void) argument;
	for( ;; ) {
		if( xQueueReceive( gpsRxQueue, &cRxedChar, ( TickType_t ) 100 ) == pdTRUE ) {
			if (line_i == MINMEA_MAX_LENGTH ) { CLEAR_LINE(); }
			line[line_i++] = cRxedChar;
			if( cRxedChar == '\n' ) {
				switch (minmea_sentence_id(line, false)) {
					case MINMEA_SENTENCE_RMC: {
						if (minmea_parse_rmc(&rmc_frame, line)) {
							if (gps_log_enable) {
								INFO("$RMC floating point degree coordinates and speed: (%f, %f) %f", minmea_tocoord(&rmc_frame.latitude), minmea_tocoord(&rmc_frame.longitude), minmea_tofloat(&rmc_frame.speed));
							}
						}
						break;
					}
					case MINMEA_SENTENCE_GGA: {
						if (minmea_parse_gga(&frame_gga, line)) {
							if(gps_log_enable) {
								INFO("$GGA: fix quality: %d", frame_gga.fix_quality);
							}
						}
						break;
					}
					case MINMEA_SENTENCE_GSA: {
						break;
					}
					case MINMEA_SENTENCE_GLL: {
						break;
					}
					case MINMEA_SENTENCE_GST: {
						break;
					}
					case MINMEA_SENTENCE_GSV: {
						if (minmea_parse_gsv(&frame_gsv, line)) {
							if (gps_log_enable ) {
								INFO("$GSV: message %d of %d", frame_gsv.msg_nr, frame_gsv.total_msgs);
								INFO("$GSV: sattelites in view: %d", frame_gsv.total_sats);
								for (int i = 0; i < 4; i++)
									INFO("$GSV: sat nr %d, elevation: %d, azimuth: %d, snr: %d dbm",
										frame_gsv.sats[i].nr,
										frame_gsv.sats[i].elevation,
										frame_gsv.sats[i].azimuth,
										frame_gsv.sats[i].snr);
								}
						}
						break;
					}
					case MINMEA_SENTENCE_VTG: {
						break;
					}
					case MINMEA_SENTENCE_ZDA: {
						break;
					}
					case MINMEA_INVALID: {
						break;
					}
					case MINMEA_UNKNOWN: {
						break;
					}

				}
				CLEAR_LINE();
			}
		}
	}
}
