#pragma once

#include "stdint.h"
#include "common/mavlink.h"

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



typedef struct __TELEMENTRY_STATS {
	uint32_t rx_good;
	uint32_t rx_bad;
	Mavlink_Messages current_messages;
} TelemetryStats;

extern TelemetryStats telemetry;


int InitTelemetry();
