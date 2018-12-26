#include "main.h"
#include "telemetry.h"
#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "queue.h"

#include "common/mavlink.h"

//#define DEBUG_MAVLINK
#define TELEMETRY_USART USART1
#define TELEMETRY_QUEUE_LEN 16

static QueueHandle_t telemetryRxQueue;
static mavlink_status_t lastStatus;
static mavlink_message_t message;
static uint16_t cRxedChar;
static uint8_t msgReceived = false;
static TimerHandle_t telemetryTimer = NULL;

void vTelemetryTimerCallback(TimerHandle_t pxTimer);

TelemetryStats telemetry;

int InitTelemetry(){
	TRACE_CHECKPOINT("telemetry init");
	telemetry.status = TELEMETRY_UNKNOWN;
	#ifdef TELEMETRY_FAKE
		telemetry.current_messages.gps_raw_int.alt = TELEMETRY_FAKE_ALT;
		telemetry.current_messages.gps_raw_int.lon = TELEMETRY_FAKE_LON;
		telemetry.current_messages.gps_raw_int.lat = TELEMETRY_FAKE_LAT;
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
	usart.USART_BaudRate = 57600;//2400;
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
	if ( xTimerStart(telemetryTimer,  ( TickType_t ) 10) == pdFAIL ) {
		ERROR("Failed to start telemetryTimer");
		return 1;
	}

	TRACE_CHECKPOINT("telemetry done");
	return 0;
}


void USART1_IRQHandler(){
	uint16_t cIn;
	BaseType_t xHigherPriorityTaskWoken;

	xHigherPriorityTaskWoken = pdFALSE;
	do {
		cIn = USART_ReceiveData (TELEMETRY_USART);
		USART_ClearITPendingBit(TELEMETRY_USART, USART_IT_RXNE);
		xQueueSendFromISR( telemetryRxQueue, &cIn, &xHigherPriorityTaskWoken );
	} while( USART_GetITStatus(TELEMETRY_USART, USART_IT_RXNE) );

	if( xHigherPriorityTaskWoken ) {
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}
}

static mavlink_status_t status;

void vTelemetryTimerCallback(TimerHandle_t pxTimer) {
	while( xQueueReceive( telemetryRxQueue, &( cRxedChar ), ( TickType_t ) 10 ) ) {
		msgReceived = mavlink_parse_char(MAVLINK_COMM_0, cRxedChar, &message, &status);

		if ( (lastStatus.packet_rx_drop_count != status.packet_rx_drop_count) ) {
			telemetry.rx_bad++;
		}
		lastStatus = status;

		if(msgReceived) {
			telemetry.rx_good++;

			telemetry.current_messages.compid = message.compid;
			telemetry.current_messages.sysid = message.sysid;

			switch (message.msgid) {
				case MAVLINK_MSG_ID_GPS_RAW_INT:
				{
					mavlink_msg_gps_raw_int_decode(&message, &(telemetry.current_messages.gps_raw_int));
					telemetry.status = TELEMETRY_OK;
					#ifdef DEBUG_MAVLINK
						INFO("MAVLINK_MSG_ID_GPS_RAW_INT %d, %d\n", telemetry.current_messages.gps_raw_int.lon, telemetry.current_messages.gps_raw_int.lat);
					#endif

					break;
				}
				/*
				case MAVLINK_MSG_ID_HEARTBEAT:
				{
					#ifdef DEBUG_MAVLINK
						INFO("MAVLINK_MSG_ID_HEARTBEAT\n");
					#endif
					mavlink_msg_heartbeat_decode(&message, &(telemetry.current_messages.heartbeat));
					break;
				}

				case MAVLINK_MSG_ID_GLOBAL_POSITION_INT:
				{
					#ifdef DEBUG_MAVLINK
						INFO("MAVLINK_MSG_ID_GLOBAL_POSITION_INT\n");
					#endif
					mavlink_msg_global_position_int_decode(&message, &(telemetry.current_messages.global_position_int));
					//telemetry.status = TELEMETRY_OK;
					break;
				}

				case MAVLINK_MSG_ID_SYS_STATUS:
				{
					//printf("MAVLINK_MSG_ID_SYS_STATUS\n");
					mavlink_msg_sys_status_decode(&message, &(current_messages.sys_status));
					current_messages.time_stamps.sys_status = get_time_usec();
					this_timestamps.sys_status = current_messages.time_stamps.sys_status;
					break;
				}

				case MAVLINK_MSG_ID_BATTERY_STATUS:
				{
					//printf("MAVLINK_MSG_ID_BATTERY_STATUS\n");
					mavlink_msg_battery_status_decode(&message, &(current_messages.battery_status));
					current_messages.time_stamps.battery_status = get_time_usec();
					this_timestamps.battery_status = current_messages.time_stamps.battery_status;
					break;
				}

				case MAVLINK_MSG_ID_RADIO_STATUS:
				{
					//printf("MAVLINK_MSG_ID_RADIO_STATUS\n");
					mavlink_msg_radio_status_decode(&message, &(current_messages.radio_status));
					current_messages.time_stamps.radio_status = get_time_usec();
					this_timestamps.radio_status = current_messages.time_stamps.radio_status;
					break;
				}

				case MAVLINK_MSG_ID_LOCAL_POSITION_NED:
				{
					//printf("MAVLINK_MSG_ID_LOCAL_POSITION_NED\n");
					mavlink_msg_local_position_ned_decode(&message, &(current_messages.local_position_ned));
					current_messages.time_stamps.local_position_ned = get_time_usec();
					this_timestamps.local_position_ned = current_messages.time_stamps.local_position_ned;
					break;
				}



				case MAVLINK_MSG_ID_POSITION_TARGET_LOCAL_NED:
				{
					//printf("MAVLINK_MSG_ID_POSITION_TARGET_LOCAL_NED\n");
					mavlink_msg_position_target_local_ned_decode(&message, &(current_messages.position_target_local_ned));
					current_messages.time_stamps.position_target_local_ned = get_time_usec();
					this_timestamps.position_target_local_ned = current_messages.time_stamps.position_target_local_ned;
					break;
				}

				case MAVLINK_MSG_ID_POSITION_TARGET_GLOBAL_INT:
				{
					//printf("MAVLINK_MSG_ID_POSITION_TARGET_GLOBAL_INT\n");
					mavlink_msg_position_target_global_int_decode(&message, &(current_messages.position_target_global_int));
					current_messages.time_stamps.position_target_global_int = get_time_usec();
					this_timestamps.position_target_global_int = current_messages.time_stamps.position_target_global_int;
					break;
				}

				case MAVLINK_MSG_ID_HIGHRES_IMU:
				{
					//printf("MAVLINK_MSG_ID_HIGHRES_IMU\n");
					mavlink_msg_highres_imu_decode(&message, &(current_messages.highres_imu));
					current_messages.time_stamps.highres_imu = get_time_usec();
					this_timestamps.highres_imu = current_messages.time_stamps.highres_imu;
					break;
				}

				case MAVLINK_MSG_ID_ATTITUDE:
				{
					//printf("MAVLINK_MSG_ID_ATTITUDE\n");
					mavlink_msg_attitude_decode(&message, &(current_messages.attitude));
					current_messages.time_stamps.attitude = get_time_usec();
					this_timestamps.attitude = current_messages.time_stamps.attitude;
					break;
				}
				 */
				default:
				{
					// printf("Warning, did not handle message id %i\n",message.msgid);
					break;
				}
			}
		}
		xTimerReset(telemetryTimer, 10);
	}
}
