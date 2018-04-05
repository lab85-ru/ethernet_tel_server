#ifndef _UPTIME_H_
#define _UPTIME_H_

#include <stdint.h>

// output structure - system uptime
typedef struct {
    uint8_t years;
	uint16_t days;
	uint8_t hours;
	uint8_t min;
	uint8_t sec;
} uptime_data_t;

void uptime(uint32_t t, uptime_data_t * uptime_data);

#endif