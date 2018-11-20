#pragma once

#include "printf.h"

#define ENABLE_SERIAL
#ifdef ENABLE_SERIAL
	#define INFO(info, ...) printf("[INFO]:  " info "\r\n", ##__VA_ARGS__)
	#define WARN(warn, ...) printf("[WARN]:  " warn "\r\n", ##__VA_ARGS__)
	#define ERROR(err, ...) printf("[ERR ]:  " err "\r\n", ##__VA_ARGS__)
#endif
