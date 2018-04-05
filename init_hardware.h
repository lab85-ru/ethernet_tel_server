/*
 * init_hardware.h
 *
 */

#ifndef INIT_HARDWARE_H_
#define INIT_HARDWARE_H_
#include <stdint.h>


// POWER FOR 22aa04 (output ON=0)
#define PWR_24AA04_OUT_PIN             GPIO_Pin_6
#define PWR_24AA04_OUT_GPIO_PORT       GPIOB
#define PWR_24AA04_OUT_GPIO_CLK        RCC_APB2Periph_GPIOB

// ETH RESET (output ON=0)
#define ETH_RESET_OUT_PIN              GPIO_Pin_5
#define ETH_RESET_OUT_GPIO_PORT        GPIOB
#define ETH_RESET_OUT_GPIO_CLK         RCC_APB2Periph_GPIOB





void init_hardware(void);
void output_gpio_byte(char b);
void pwr_on_24aa04(void);
void pwr_off_24aa04(void);
void eth_reset_on(void);
void eth_reset_off(void);

#endif /* PRVSETUPHARDWARE_H_ */
