#include "main.h"
#include "settings.h"
#include "FreeRTOS.h"
#include "FreeRTOS_CLI.h"
#include "task.h"
#include <stdio.h>
#include <stdbool.h>


static BaseType_t prvSettingsGetCommand( int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString ) {
	(void) xWriteBufferLen;
	const char *pcParameter1;
	BaseType_t xParameter1Length;
	pcParameter1 = FreeRTOS_CLIGetParameter ( pcCommandString, 1, &xParameter1Length);
	uint32_t value = settingsGetInt32(SettingStrToSetting(pcParameter1));

	snprintf( (char*)pcWriteBuffer, xWriteBufferLen, "%s=%d\r\n", pcParameter1, (int)value);
	return pdFALSE;
}

static BaseType_t prvSettingsSetCommand( int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString ) {
	(void) xWriteBufferLen;
	int8_t *pcParameter1, *pcParameter2;
	BaseType_t xParameter1Length,xParameter2Length;
	pcParameter1 = FreeRTOS_CLIGetParameter ( pcCommandString, 1, &xParameter1Length);
	char settingStr[16]= {0x00};
	strncpy(settingStr, pcParameter1, xParameter1Length);

	pcParameter2 = FreeRTOS_CLIGetParameter ( pcCommandString, 2, &xParameter2Length);
	settingsSetInt32(SettingStrToSetting(settingStr), atoi(pcParameter2));

	snprintf( (char*)pcWriteBuffer, xWriteBufferLen, "%s=%d\r\n", settingStr, atoi(pcParameter2));
	return pdFALSE;
}


static BaseType_t prvSettingsDumpCommand( int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString ) {
	(void) xWriteBufferLen;

	for (int setting = VERSION+1; setting < UNKNOWN;  setting++){
		 char tmp[64]={0};
		 uint8_t tmpLen = sprintf( tmp, "set %s %d\r\n", SETTING_STR[setting], settingsGetInt32(setting));
		 strncat( pcWriteBuffer, tmp, tmpLen );
	}

	return pdFALSE;
}


static const CLI_Command_Definition_t xGetCommand = {
	"get",
	"\r\nget <setting>:\r\n Get setting value\r\n",
	prvSettingsGetCommand,
	1
};

static const CLI_Command_Definition_t xSetCommand = {
	"set",
	"\r\nset <setting> <value>:\r\n Set setting value\r\n",
	prvSettingsSetCommand,
	2
};

static const CLI_Command_Definition_t xDumpCommand = {
	"dump",
	"\r\ndump:\r\n Dump settings\r\n",
	prvSettingsDumpCommand,
	0
};

int InitSettings(){
	FreeRTOS_CLIRegisterCommand( &xGetCommand );
	FreeRTOS_CLIRegisterCommand( &xSetCommand );
	FreeRTOS_CLIRegisterCommand( &xDumpCommand );

	if (settingsGetInt32(VERSION) != SETTING_DEF[VERSION] ) {
		for( SETTING setting=SETTING_COUNT; setting > VERSION; --setting) {
			if (settingsSetInt32(setting-1, SETTING_DEF[setting-1]) != 0 ){
				return 1;
			}
		}
	}
	return 0;
}

int32_t settingsGetInt32(SETTING setting){
	uint32_t value;
	EE_Read(sizeof(int32_t)*setting, &value);
	return value;
}

bool settingsSetInt32(SETTING setting, int32_t value){
	bool res = !EE_Write(sizeof(int32_t)*setting, value);
	return res;
}

bool settingsSetInt32_RTOS(SETTING setting, int32_t value){
	taskENTER_CRITICAL();
	bool res = settingsSetInt32(setting, value);
	taskEXIT_CRITICAL();
	return res;
}
