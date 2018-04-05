#include <stdint.h>
#include "sys_timer.h"
#include "stm32f10x.h"
#include "time_hal.h"
#include "printf_hal.h"

#ifndef DEBUG_TIME
#define DEBUG_TIME 0
#endif

extern uint32_t time_get_ms_counter( void );

//******************************************************************************
// Delay ms
//******************************************************************************
void delay_ms( const uint32_t delay )
{
    uint32_t t = time_get_ms_counter();

    while (calcul_time_out( time_get_ms_counter(), t, delay ) == TIR_WAIT){
        __WFI();
        __NOP();
    }
}

//******************************************************************************
// Delay sec
//******************************************************************************
void delay_s( const uint32_t delay )
{
    uint32_t t = time_get_ms_counter();

    while (calcul_time_out( time_get_ms_counter(), t, delay * 1000 ) == TIR_WAIT){
        __WFI();
        __NOP();
    }
}

//******************************************************************************
// Poluchenie Statusa - skolko vremeni prohlo s poslednego sobitiya.
// Status vichislaetcha kak raznost dT = (T tekushee)-(T sobitiya(prohloe))
// dalee if dT >= INTERVAL then return TIR_OUT else return TIR_WAIT
// Time = sec,ms & etc
//
// t_start - vrema nachala otchuta
// t_delay - interval 
// t_cur   - tekushee vrema
//
//******************************************************************************
tir_e calcul_time_out( const uint32_t t_cur, const uint32_t t_start, const uint32_t t_delay )
{
	uint32_t t_delta;
	
	if (DEBUG_TIME) printf("Time start = %ul\n", t_start);
	if (DEBUG_TIME) printf("time cur   = %ul\n", t_cur);
	if (t_cur >= t_start){
		t_delta = t_cur - t_start;
	}else{
	        t_delta = (uint32_t)0xffffffffUL - t_start + t_cur;
	}
	if (DEBUG_TIME) printf("\nTIME DELTA=%ul   time interval =%ul\n", t_delta, t_delay);
	if (t_delta >= t_delay)
		return TIR_OUT;
	else
		return TIR_WAIT;
}
