#pragma once

#include <stdint.h>
#include <string.h>
#include <stdbool.h>

typedef enum {
	VERSION, // version should be first
	SERIAL_BAUD,
	GPS_BAUD,
	UNKNOWN, // should be pre last
	SETTING_COUNT // should be last
} SETTING;

static uint32_t SETTING_DEF[SETTING_COUNT-1] = {
	2,
	115200,
	9600,
};

static char* SETTING_STR[SETTING_COUNT-1] = {
	"version",
	"serial_baud",
	"gps_baud",
};


#ifdef __cplusplus
extern "C" {
#endif
	static int SettingStrToSetting(char* str){
		for (int setting = VERSION; setting < SETTING_COUNT;  setting++){
			if (strcmp(SETTING_STR[setting], str) == 0) return setting;
		}
		return UNKNOWN;
	}

	int InitSettings();
	int32_t settingsGetInt32(SETTING setting);
	bool settingsSetInt32(SETTING setting, int32_t value);
	bool settingsSetInt32_RTOS(SETTING setting, int32_t value);
#ifdef __cplusplus
}
#endif

