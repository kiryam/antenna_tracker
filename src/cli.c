#include "FreeRTOS.h"
#include "FreeRTOS_CLI.h"
#include "task.h"
#include "serial.h"
#include "string.h"

#define MAX_INPUT_LENGTH    50
#define MAX_OUTPUT_LENGTH   configCOMMAND_INT_MAX_OUTPUT_SIZE

#define COMMAND_TASKS
static int8_t pcOutputString[ MAX_OUTPUT_LENGTH ], pcInputString[ MAX_INPUT_LENGTH ];
static uint16_t cRxedChar;

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


void initCLI(){
	#ifdef COMMAND_TASKS
		FreeRTOS_CLIRegisterCommand( &xTasksCommand );
	#endif
}

void vCommandConsoleTask( void *pvParameters ) {
	( void ) pvParameters;
	uint16_t cInputIndex = 0;
	BaseType_t xMoreDataToFollow;

    for( ;; ) {
    	if( xQueueReceive( serialRxQueue, &( cRxedChar ), ( TickType_t ) 100 ) ) {
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
}
