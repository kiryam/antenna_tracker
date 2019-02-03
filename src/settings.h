#pragma once

#include <stdint.h>
#include <string.h>
#include <stdbool.h>

typedef enum {
	VERSION, // version should be first
	SERIAL_BAUD,
	GPS_BAUD,
	TELEMETRY_BAUD,
	TELEMETRY_TYPE,
	TELEMETRY_FAKE,
	TELEMETRY_FAKE_LAT,
	TELEMETRY_FAKE_LON,
	TELEMETRY_FAKE_ALT,
	STEPPER_ENABLED,
	SERVO_ENABLED,
	GUI_REFRESH_INTERVAL,

	UNKNOWN, // should be pre last
	SETTING_COUNT // should be last
} SETTING;

static int32_t SETTING_DEF[SETTING_COUNT-1] = {
	8,
	115200,
	9600,
	57600,
	0, // 0 - Mavlink, 1- LTM
	1,
	558940620,
	372514570,
	1000*10,
	1,
	1,
	1000
};

static char* SETTING_STR[SETTING_COUNT-1] = {
	"version",
	"serial_baud",
	"gps_baud",
	"telemetry_baud",
	"telemetry_type",
	"telemetry_fake",
	"telemetry_fake_lat",
	"telemetry_fake_lon",
	"telemetry_fake_alt",
	"stepper_enabled",
	"servo_enabled",
	"gui_refresh_interval"
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

