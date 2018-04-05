/*
 * prvsetuphardware.c
 *
 *  Created on: 06.09.2010
 *      Author: Администратор
 */

#include <stdint.h>
#include <string.h>
#include "printf_hal.h"
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "main.h"
#include "board.h"

#include "FreeRTOS.h"
#include "task.h"

#include "queue_buf.h"
#include "i2c.h"
#include "enc28j60.h"
#include "mac.h"
#include "uip.h"
#include "uip_arp.h"
#include "env_config.h"
#include "adc.h"
#include "init_hardware.h"

const uint16_t out_gpio_array[] = {OUT_PIN_0, OUT_PIN_1, OUT_PIN_2, OUT_PIN_3, OUT_PIN_4, OUT_PIN_5, OUT_PIN_6, OUT_PIN_7}; 

extern char txt_buf[];
extern struct queue_buffer consol_rx;
extern uint8_t consol_rx_buf[ QUEUE_CONSOL_RX_SIZE ];
extern struct uip_eth_addr mac_addr;
extern device_status_t device_status;

void init_hardware(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    unsigned int i;
    volatile uint32_t j;

    // UART Init (CONSOL)
    USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;

	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

	stm32_uart_init(COM1);
	stm32_uart_set(COM1, &USART_InitStructure, ENABLE);

  	// инит консоль
	consol_rx.queue = consol_rx_buf;
	consol_rx.in = 0;
	consol_rx.out = 0;
	consol_rx.len = QUEUE_CONSOL_RX_SIZE;
    
    // Очищаем буфер АЦП от случайных данных.
    for (i=0;i<ADC_CHANNEL_QUANTITY; i++) 
        device_status.adc_converted_value[ i ] = 0;
        
    I2C_Configuration();

    // configure pin power On-OFF for 24AA04
	RCC_APB2PeriphClockCmd(PWR_24AA04_OUT_GPIO_CLK, ENABLE);
	GPIO_InitStructure.GPIO_Pin = PWR_24AA04_OUT_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(PWR_24AA04_OUT_GPIO_PORT, &GPIO_InitStructure);
    pwr_off_24aa04();

    // configure ETH reset
	RCC_APB2PeriphClockCmd(ETH_RESET_OUT_GPIO_CLK, ENABLE);
	GPIO_InitStructure.GPIO_Pin = ETH_RESET_OUT_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(ETH_RESET_OUT_GPIO_PORT, &GPIO_InitStructure);
    eth_reset_off();

    eth_reset_on();
	for (j=0; j<1000000; j++) {	}
    eth_reset_off();
	for (j=0; j<1000000; j++) {	}
        
    pwr_on_24aa04();
	for (j=0; j<1000000; j++) {	}
    // read MAC from 24aa04
    mac_read( mac_addr.addr );
    pwr_off_24aa04();
	for (j=0; j<1000000; j++) {	}       
       
    // Настройка Цифровых Входов и выходов(GPIO которые выходят наружу !!!)
        
    // switch clock to GPIO_INPUT
	RCC_APB2PeriphClockCmd( IN_GPIO_CLK, ENABLE );

	GPIO_InitStructure.GPIO_Pin = IN_PIN_ALL;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init( IN_GPIO_PORT, &GPIO_InitStructure );
        
       	// switch clock to GPIO_OUTPUT
	RCC_APB2PeriphClockCmd( OUT_GPIO_CLK, ENABLE );

	GPIO_InitStructure.GPIO_Pin = OUT_PIN_ALL;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init( OUT_GPIO_PORT, &GPIO_InitStructure );

    adc_init();     

    printf_d("Start IWDG(watchdog) interval 280ms.\r\n");
	/* IWDG timeout equal to 280 ms (the timeout may varies due to LSI frequency dispersion) */
	/* Enable write access to IWDG_PR and IWDG_RLR registers */
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);

	/* IWDG counter clock: 40KHz(LSI) / 32 = 1.25 KHz */
	IWDG_SetPrescaler(IWDG_Prescaler_32);

	/* Set counter reload value to 349 */
	IWDG_SetReload(349);

	/* Reload IWDG counter */
	IWDG_ReloadCounter();

	/* Enable IWDG (the LSI oscillator will be enabled by hardware) */
	IWDG_Enable();
}

//------------------------------------------------------------------------------
// побитовый вывод байта в порт GPIO
//------------------------------------------------------------------------------
void output_gpio_byte(char b)
{
    int i;
        
    for (i=0;i<8;i++){
        if ((1<<i) & b)
            OUT_GPIO_PORT->BSRR = out_gpio_array[i];		// set gpio=1
        else
            OUT_GPIO_PORT->BRR = out_gpio_array[i];		    // set gpio=0
    }
}

//------------------------------------------------------------------------------
// Подача питания на микросхему 24aa04
//------------------------------------------------------------------------------
void pwr_on_24aa04(void)
{
    PWR_24AA04_OUT_GPIO_PORT->BRR = PWR_24AA04_OUT_PIN;
}

//------------------------------------------------------------------------------
// Снятие питания с микросхемы 24aa04
//------------------------------------------------------------------------------
void pwr_off_24aa04(void)
{
    PWR_24AA04_OUT_GPIO_PORT->BSRR = PWR_24AA04_OUT_PIN;
}

//------------------------------------------------------------------------------
// reset ON(low) ETHERNET
//------------------------------------------------------------------------------
void eth_reset_on(void)
{
    ETH_RESET_OUT_GPIO_PORT->BRR = ETH_RESET_OUT_PIN;
}

//------------------------------------------------------------------------------
// reset OFF(high) ETHERNET
//------------------------------------------------------------------------------
void eth_reset_off(void)
{
    ETH_RESET_OUT_GPIO_PORT->BSRR = ETH_RESET_OUT_PIN;
}
