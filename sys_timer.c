#include "stm32f10x.h"
#include "stm32f10x_iwdg.h"
#include "sys_timer.h"

static volatile uint32_t sys_tick_counter    = 0;  // ������� ���������� �������
static volatile uint32_t tick_sec_counter    = 0;  // ������� ������
static volatile uint32_t tick_1000ms_counter = 0;  // ������� 1000�� ����������

//------------------------------------------------------------------------------
// ���������� ������� �������� ������� ������
//------------------------------------------------------------------------------
uint32_t time_get_sec_counter( void )
{
	return tick_sec_counter;
}


//------------------------------------------------------------------------------
// ���������� ������� �������� ���������� �������, ������� ms
//------------------------------------------------------------------------------
uint32_t time_get_ms_counter( void )
{
	return sys_tick_counter;
}

//------------------------------------------------------------------------------
// ���������� ���������� �� ���������� �������
//------------------------------------------------------------------------------
void sys_tick_handler( void )
{
	/* Reload IWDG counter */
	IWDG_ReloadCounter();
        sys_tick_counter++;
        tick_1000ms_counter++;
        if (tick_1000ms_counter == 1000){
            tick_1000ms_counter = 0;
            tick_sec_counter++;       // sec + 1
        }
}

//------------------------------------------------------------------------------
// ��������� ���������� �������
// configCPU_CLOCK_HZ �������� ������� ����
// configTICK_RATE_HZ ������� ����� �������
//------------------------------------------------------------------------------
//void sys_timer_setup( void )
//{
//        SysTick_Config( (configCPU_CLOCK_HZ) / (configTICK_RATE_HZ) );
//}

