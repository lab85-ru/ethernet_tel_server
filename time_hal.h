#ifndef TIME_HAL_H_
#define TIME_HAL_H_

#include <stdint.h>
#include "sys_timer.h"

#define DELAY_10MS     (10)
#define DELAY_100MS    (100)
#define DELAY_250MS    (250)
#define DELAY_500MS    (500)
#define DELAY_1000MS   (1000)


#define DELAY_1S       (1)
#define DELAY_5S       (5)
#define DELAY_10S      (10)

typedef enum {TIR_WAIT=0, TIR_OUT=1} tir_e;                          // result return for delta intervals

void delay_ms( const uint32_t delay );
void delay_s( const uint32_t delay );
tir_e calcul_time_out( const uint32_t t_cur, const uint32_t t_start, const uint32_t t_delay );

#endif /*TIME_HAL_H_*/
