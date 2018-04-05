#include <stdint.h>
#include <stddef.h>
#include "uptime.h"

//=============================================================================
// input:
// t - input time uint32_t (sec)
// output:
// = 0 - ok
// != 0 - error
// structure uptime_data
//=============================================================================
void uptime(uint32_t t, uptime_data_t * uptime_data)
{
    const uint32_t YY = 365 * 24 * 60 * 60; // sec for 1 Year.
	const uint32_t DD = 24 * 60 * 60;       // sec for 1 Days.
	const uint32_t HH = 60 * 60;            // sec for 1 hour.

	uint32_t tt = t;
    uint32_t years;
	uint32_t days;
	uint32_t hours;
	uint32_t min;
	uint32_t sec;

	if (uptime_data == NULL){
		return;
	}

	years = tt / YY;
	tt = tt % YY;

	days = tt / DD;
    tt = tt % DD;

	hours = tt / HH;
	tt = tt % HH;

	min = tt / 60;
	sec = tt % 60;

	uptime_data->years = (uint8_t)years;
	uptime_data->days  = (uint16_t)days;
	uptime_data->hours = (uint8_t)hours;
	uptime_data->min   = (uint8_t)min;
	uptime_data->sec   = (uint8_t)sec;

	return;
}
