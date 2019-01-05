#include "main.h"
#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "FreeRTOS_CLI.h"
#include "timers.h"
#include "queue.h"
#include "stdbool.h"
#include "minmea.h"
#include "serial.h"
#include "string.h"
#include "gps.h"
#include "settings.h"

#define GPS_USART USART3
#define GPS_USART_IRQ USART3_IRQn
#define GPS_QUEUE_LEN 10


void vGPSTimerCallback(TimerHandle_t pxTimer);

static QueueHandle_t gpsRxQueue;
static char line[MINMEA_MAX_LENGTH];
static uint16_t line_i;
static uint16_t cRxedChar;
static struct minmea_sentence_rmc rmc_frame;
static struct minmea_sentence_gsv frame_gsv;
static struct minmea_sentence_gga frame_gga;
static bool gps_log_enable;
static TimerHandle_t gpsTimer = NULL;

GPSStatus gps;

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
	"gps_log <ENABLE|DISABLE>:\r\n Enable or disable gps output logging\r\n",
	prvGPSLogCommand,
	1
};

int gpsInit(){
	TRACE_CHECKPOINT("gps init");
	gpsRxQueue = xQueueCreate( GPS_QUEUE_LEN, sizeof( uint16_t ) );
	if( gpsRxQueue == NULL ) {
		ERROR("GPS init queue error\n");
		return 1;
	}

	if (FreeRTOS_CLIRegisterCommand( &xTasksCommand ) != pdPASS ) {
		ERROR("GPS cli init error\n");
		return 1;
	}

	#ifdef GPS_FAKE
		gps.fix = MINMEA_GPGSA_FIX_3D;
		gps.lat = GPS_FAKE_LAT;
		gps.lon = GPS_FAKE_LON;
		gps.alt = GPS_FAKE_ALT;
	#endif

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
	usart.USART_BaudRate = settingsGetInt32(GPS_BAUD);
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
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = configLIBRARY_LOWEST_INTERRUPT_PRIORITY;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	NVIC_EnableIRQ(GPS_USART_IRQ);

	gpsTimer = xTimerCreate( "gpsTimer", 500 ,pdTRUE,0, vGPSTimerCallback);
	if (gpsTimer == NULL) {
		ERROR("Failed to create gpsTimer");
		return 1;
	}
	if ( xTimerStart(gpsTimer,  ( TickType_t ) 10) == pdFAIL ) {
		ERROR("Failed to start gpsTimer");
		return 1;
	}

	TRACE_CHECKPOINT("gps init done");
	return 0;
}

void USART3_IRQHandler(){
	if(USART_GetITStatus(GPS_USART, USART_IT_RXNE) != RESET) {
		uint16_t cIn;
		BaseType_t xHigherPriorityTaskWoken;

		xHigherPriorityTaskWoken = pdFALSE;
		do {
			cIn = USART_ReceiveData (GPS_USART);
			xQueueSendFromISR( gpsRxQueue, &cIn, &xHigherPriorityTaskWoken );
		} while( USART_GetFlagStatus(GPS_USART, USART_FLAG_RXNE) );

		USART_ClearITPendingBit(GPS_USART, USART_IT_RXNE);

		if( xHigherPriorityTaskWoken ) {
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		}
	}
}

void vGPSTimerCallback(TimerHandle_t pxTimer) {
	(void) pxTimer;
	while( xQueueReceive( gpsRxQueue, &cRxedChar, ( TickType_t ) 10 ) ) {
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

						#ifndef GPS_FAKE
							gps.fix = (uint8_t)frame_gga.fix_quality;
							gps.lat = minmea_tocoord(&rmc_frame.latitude)*10000000;
							gps.lon = minmea_tocoord(&rmc_frame.longitude)*10000000;
							gps.alt = frame_gga.altitude.value;
						#endif
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
