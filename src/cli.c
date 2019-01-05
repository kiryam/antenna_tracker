#include "FreeRTOS.h"
#include "FreeRTOS_CLI.h"
#include "timers.h"
#include "task.h"
#include "serial.h"
#include "string.h"
#include "main.h"

#define MAX_INPUT_LENGTH    50
#define MAX_OUTPUT_LENGTH   configCOMMAND_INT_MAX_OUTPUT_SIZE

static int8_t pcOutputString[ MAX_OUTPUT_LENGTH ], pcInputString[ MAX_INPUT_LENGTH ];
static uint16_t cRxedChar;
static TimerHandle_t cliTimer = NULL;

void vCLITimerCallback(TimerHandle_t pxTimer);

#ifdef COMMAND_TASKS
	static BaseType_t prvTaskStatsCommand( int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString ) {
		(void) xWriteBufferLen;
		(void) pcCommandString;

		const char *const pcHeader = "Task          State  Priority  Stack	#\r\n************************************************\r\n";
		strcpy( pcWriteBuffer, pcHeader );
		vTaskList( pcWriteBuffer+ strlen(pcHeader) );
		return pdFALSE;
	}
	static const CLI_Command_Definition_t xTasksCommand = {
		"tasks",
		"\r\ntasks:\r\n Get list of active tasks\r\n",
		prvTaskStatsCommand,
		0
	};
#endif

int initCLI(){
	#ifdef COMMAND_TASKS
		FreeRTOS_CLIRegisterCommand( &xTasksCommand );
	#endif

	cliTimer = xTimerCreate( "CLITimer",100,pdTRUE,0, vCLITimerCallback);
	if (cliTimer == NULL) {
		ERROR("Failed to create CLITimer");
		return 1;
	}
	if ( xTimerStart(cliTimer,  ( TickType_t ) 10) == pdFAIL ) {
		ERROR("Failed to start CLITimer");
		return 1;
	}

	return 0;
}

void vCLITimerCallback(TimerHandle_t pxTimer) {
	(void) pxTimer;
	uint16_t cInputIndex = 0;
	BaseType_t xMoreDataToFollow;
	while( xQueueReceive( serialRxQueue, &( cRxedChar ), ( TickType_t ) 10 ) ) {
		if( cRxedChar == '\n' ) {
			serialSendStr("\r\n");
			do {
				xMoreDataToFollow = FreeRTOS_CLIProcessCommand ( pcInputString, pcOutputString, MAX_OUTPUT_LENGTH);
				serialSendStr(pcOutputString);
			} while( xMoreDataToFollow != pdFALSE );

			cInputIndex = 0;
			memset( pcInputString, 0x00, MAX_INPUT_LENGTH );
		} else {
			if( cRxedChar == '\r' ) {

			} else if( cRxedChar == '\b' ) {
				if( cInputIndex > 0 ) {
					cInputIndex--;
					pcInputString[ cInputIndex ] = '\0';
				}
			} else {
				if( cInputIndex < MAX_INPUT_LENGTH ) {
					pcInputString[ cInputIndex ] = cRxedChar;
					cInputIndex++;
					char tmp[1];
					tmp[0] = cRxedChar;
					serialSendBytes(tmp, 1);
				}
			}
		}
	}
}
