#pragma once

#define SCREEN_SWITCH_TIMEOUT 3000

static enum screen_type {
		SCREEN_TELEMETRY,
		SCREEN_SYSTEM,
		SCREEN_GPS,
		SCREEN_SERVO,

		SCREEN_ENUM_LEN
	} active_screen;
