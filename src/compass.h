#pragma once

#ifdef __cplusplus
extern "C" {
#endif

	void compassInitTask(void *pvParameters);
	int InitCompass();
	uint16_t GetHomeBearing();

#ifdef __cplusplus
}
#endif
