#pragma once


#ifdef __cplusplus
extern "C" {
#endif

	int gpsInit();
	void processGPS();

#ifdef __cplusplus
}
#endif

typedef struct __GPS_STATUS {
	uint8_t fix;
	int32_t lon;
	int32_t lat;
	int32_t alt;
	uint16_t sats;
} GPSStatus;

extern GPSStatus gps;
