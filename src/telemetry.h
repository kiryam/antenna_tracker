#pragma once

#include "stdint.h"
#include "common/mavlink.h"
#include "FreeRTOS.h"
#include "queue.h"
typedef struct __Mavlink_Messages {

	int sysid;
	int compid;

	// Heartbeat
	mavlink_heartbeat_t heartbeat;

	// System Status
//	mavlink_sys_status_t sys_status;

	// Battery Status
//	mavlink_battery_status_t battery_status;

	// Radio Status
//	mavlink_radio_status_t radio_status;

	// Local Position
//	mavlink_local_position_ned_t local_position_ned;

	// Global Position
	mavlink_global_position_int_t global_position_int;

	mavlink_gps_raw_int_t gps_raw_int;

	// Local Position Target
//	mavlink_position_target_local_ned_t position_target_local_ned;

	// Global Position Target
	//mavlink_position_target_global_int_t position_target_global_int;

	// HiRes IMU
	//mavlink_highres_imu_t highres_imu;

	// Attitude
	//mavlink_attitude_t attitude;

	// System Parameters?


	// Time Stamps
	//Time_Stamps time_stamps;

	//void
	//reset_timestamps()
	//{
		//time_stamps.reset_timestamps();
	//}

} Mavlink_Messages;

#define TELEMETRY_UNKNOWN 0
#define TELEMETRY_OK 1
#define TELEMETRY_BAD 2

typedef struct __TELEMENTRY_STATS {
	uint32_t rx_good;
	uint32_t rx_bad;
	int32_t lat;
	int32_t lon;
	int32_t alt;
	uint16_t groundspeed_ms;
	uint8_t status;
	uint8_t satellites_visible;
	uint8_t fix_type;
} TelemetryStats;

extern TelemetryStats telemetry;
extern QueueHandle_t telemetryRxQueue;

#ifdef __cplusplus
extern "C" {
#endif

	int InitTelemetry();
	void processTelemetry();

#ifdef __cplusplus
}
#endif

