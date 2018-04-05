/**
  ******************************************************************************
  * @file    stm32100b_eval.c
  * @author  MCD Application Team
  * @version V4.3.0
  * @date    10/15/2010
  * @brief   This file provides
  *            - set of firmware functions to manage Leds, push-button and COM ports
  *            - low level initialization functions for SD card (on SPI), SPI serial
  *              flash (sFLASH) and temperature sensor (LM75)
  *          available on STM32100B-EVAL evaluation board from STMicroelectronics. 
  ******************************************************************************
  * @copy
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2010 STMicroelectronics</center></h2>
  */ 
  
/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "stm32f10x_conf.h"
#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "task.h"
#include "board.h"

#ifndef DEBUG
#define DEBUG 0
#endif

extern void printf_dos(const char *args, ...);
extern void printf_d(const char *args, ...);

USART_TypeDef* const COM_USART[COMn] = {H_COM1, H_COM2, H_COM3};

GPIO_TypeDef* const COM_TX_PORT[COMn] = {H_COM1_TX_GPIO_PORT, H_COM2_TX_GPIO_PORT, H_COM3_TX_GPIO_PORT};
GPIO_TypeDef* const COM_RX_PORT[COMn] = {H_COM1_RX_GPIO_PORT, H_COM2_RX_GPIO_PORT, H_COM3_RX_GPIO_PORT};
const uint32_t COM_USART_CLK[COMn] = {H_COM1_CLK, H_COM2_CLK, H_COM3_CLK};
const uint32_t COM_TX_PORT_CLK[COMn] = {H_COM1_TX_GPIO_CLK, H_COM2_TX_GPIO_CLK, H_COM3_TX_GPIO_CLK};
const uint32_t COM_RX_PORT_CLK[COMn] = {H_COM1_RX_GPIO_CLK, H_COM2_RX_GPIO_CLK, H_COM3_RX_GPIO_CLK};
const uint16_t COM_TX_PIN[COMn] = {H_COM1_TX_PIN, H_COM2_TX_PIN, H_COM3_TX_PIN};
const uint16_t COM_RX_PIN[COMn] = {H_COM1_RX_PIN, H_COM2_RX_PIN, H_COM3_RX_PIN};

GPIO_TypeDef* const GPIO_PORT[LEDn] = {LEDR_GPIO_PORT, LEDG_GPIO_PORT};
const uint16_t GPIO_PIN[LEDn] = {LEDR_PIN, LEDG_PIN};
const uint32_t GPIO_CLK[LEDn] = {LEDR_GPIO_CLK, LEDG_GPIO_CLK};


/**
  * @brief  Configures LED GPIO.
  * @param  Led: Specifies the Led to be configured. 
  *   This parameter can be one of following parameters:
  *     @arg LED1
  *     @arg LED2
  *     @arg LED3
  *     @arg LED4
  * @retval None
  */
void STM_EVAL_LEDInit(Led_TypeDef Led)
{
  GPIO_InitTypeDef  GPIO_InitStructure;
  
  /* Enable the GPIO_LED Clock */
  RCC_APB2PeriphClockCmd(GPIO_CLK[Led], ENABLE);

  /* Configure the GPIO_LED pin */
  GPIO_InitStructure.GPIO_Pin = GPIO_PIN[Led];
  
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIO_PORT[Led], &GPIO_InitStructure);
}

/**
  * @brief  Turns selected LED On.
  * @param  Led: Specifies the Led to be set on. 
  *   This parameter can be one of following parameters:
  *     @arg LED1
  *     @arg LED2
  *     @arg LED3
  *     @arg LED4  
  * @retval None
  */
void STM_EVAL_LEDOn(Led_TypeDef Led)
{
  GPIO_PORT[Led]->BRR = GPIO_PIN[Led];   
}

/**
  * @brief  Turns selected LED Off.
  * @param  Led: Specifies the Led to be set off. 
  *   This parameter can be one of following parameters:
  *     @arg LED1
  *     @arg LED2
  *     @arg LED3
  *     @arg LED4 
  * @retval None
  */
void STM_EVAL_LEDOff(Led_TypeDef Led)
{
  GPIO_PORT[Led]->BSRR = GPIO_PIN[Led];   
}

/**
  * @brief  Toggles the selected LED.
  * @param  Led: Specifies the Led to be toggled. 
  *   This parameter can be one of following parameters:
  *     @arg LED1
  *     @arg LED2
  *     @arg LED3
  *     @arg LED4  
  * @retval None
  */
void STM_EVAL_LEDToggle(Led_TypeDef Led)
{
  GPIO_PORT[Led]->ODR ^= GPIO_PIN[Led];
}



void com_init(uint8_t port_num, uint32_t baud)
{
	USART_InitTypeDef USART_InitStructure;

	if (port_num<=4){

		USART_InitStructure.USART_BaudRate = baud;
		USART_InitStructure.USART_WordLength = USART_WordLength_8b;
		USART_InitStructure.USART_StopBits = USART_StopBits_1;
		USART_InitStructure.USART_Parity = USART_Parity_No;
		USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
		USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

		if (baud)
			stm32_uart_set( (COM_TypeDef)port_num, &USART_InitStructure, ENABLE);
		else
			stm32_uart_set( (COM_TypeDef)port_num, &USART_InitStructure, DISABLE);
	}
}



void com_tx_start( uint8_t port_num )
{
	if (port_num<=4){
		if (DEBUG) printf_dos("com_tx_start port=%d\r\n",port_num);
		taskDISABLE_INTERRUPTS();
		USART_ITConfig( COM_USART[ port_num ], USART_IT_TXE, ENABLE);
		taskENABLE_INTERRUPTS();
	}
}




void nvic_config_com1(FunctionalState st)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	#ifdef SOFTOFFSET
	NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x20000);
	#endif
	/* Enable the USART1 Interrupt */
  	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  	NVIC_InitStructure.NVIC_IRQChannelCmd = st;
  	NVIC_Init(&NVIC_InitStructure);
}

void nvic_config_com2(FunctionalState st)
{
  	NVIC_InitTypeDef NVIC_InitStructure;
	#ifdef SOFTOFFSET
	NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x20000);
	#endif
  	/* Enable the USART2 Interrupt */
  	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  	NVIC_InitStructure.NVIC_IRQChannelCmd = st;
  	NVIC_Init(&NVIC_InitStructure);
}

void nvic_config_com3(FunctionalState st)
{
  	NVIC_InitTypeDef NVIC_InitStructure;
	#ifdef SOFTOFFSET
	NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x20000);
	#endif
  	/* Enable the USART3 Interrupt */
  	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  	NVIC_InitStructure.NVIC_IRQChannelCmd = st;
  	NVIC_Init(&NVIC_InitStructure);
}



/**
  * Configures COM port.
  * set ENABLE-DISABLE
  */
void stm32_uart_set(COM_TypeDef COM, USART_InitTypeDef* USART_InitStruct, FunctionalState st)
{

	if( USART_InitStruct->USART_BaudRate != 0 && st == ENABLE ){
		portDISABLE_INTERRUPTS();
		/* USART configuration */
		USART_Init(COM_USART[COM], USART_InitStruct);
		/* Enable USART */
		USART_Cmd(COM_USART[COM], ENABLE);
		/* Enable Int to RX char */
		USART_ITConfig(COM_USART[COM], USART_IT_RXNE, ENABLE);
		portENABLE_INTERRUPTS();
		return;
	}

	portDISABLE_INTERRUPTS();
	/* DISABLE Int to TX-RX char */
  	USART_ITConfig( COM_USART[ COM ], USART_IT_RXNE, DISABLE);
	USART_ITConfig( COM_USART[ COM ], USART_IT_TXE, DISABLE);
	USART_Cmd(COM_USART[ COM ], DISABLE);
	portENABLE_INTERRUPTS();
	return;
}


void stm32_uart_init(COM_TypeDef COM)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  switch (COM){
  case COM1:
	  nvic_config_com1(ENABLE);
	  /* Enable GPIO clock */
	  RCC_APB2PeriphClockCmd(COM_TX_PORT_CLK[COM] | COM_RX_PORT_CLK[COM] | RCC_APB2Periph_AFIO, ENABLE);
	  /* Enable UART clock */
	  RCC_APB2PeriphClockCmd(COM_USART_CLK[COM], ENABLE);
	  /* Configure USART Tx as alternate function push-pull */
	  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	  GPIO_InitStructure.GPIO_Pin = COM_TX_PIN[COM];
	  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	  GPIO_Init(COM_TX_PORT[COM], &GPIO_InitStructure);

	  /* Configure USART Rx as input floating */
	  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	  GPIO_InitStructure.GPIO_Pin = COM_RX_PIN[COM];
	  GPIO_Init(COM_RX_PORT[COM], &GPIO_InitStructure);

	  break;
  case COM2:
	  nvic_config_com2(ENABLE);
	  /* Enable GPIO clock */
	  RCC_APB2PeriphClockCmd(COM_TX_PORT_CLK[COM] | COM_RX_PORT_CLK[COM] | RCC_APB2Periph_AFIO, ENABLE);
	  /* Enable UART clock */
	  RCC_APB1PeriphClockCmd(COM_USART_CLK[COM], ENABLE);
	  /* Configure USART Tx as alternate function push-pull */
	  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	  GPIO_InitStructure.GPIO_Pin = COM_TX_PIN[COM];
	  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	  GPIO_Init(COM_TX_PORT[COM], &GPIO_InitStructure);

	  /* Configure USART Rx as input floating */
	  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	  GPIO_InitStructure.GPIO_Pin = COM_RX_PIN[COM];
	  GPIO_Init(COM_RX_PORT[COM], &GPIO_InitStructure);

	  break;
  case COM3:
	  nvic_config_com3(ENABLE);
	  /* Enable GPIO clock */
	  RCC_APB2PeriphClockCmd(COM_TX_PORT_CLK[COM] | COM_RX_PORT_CLK[COM] | RCC_APB2Periph_AFIO, ENABLE);
	  /* Enable UART clock */
	  RCC_APB1PeriphClockCmd(COM_USART_CLK[COM], ENABLE);
	  /* Configure USART Tx as alternate function push-pull */
	  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	  GPIO_InitStructure.GPIO_Pin = COM_TX_PIN[COM];
	  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	  GPIO_Init(COM_TX_PORT[COM], &GPIO_InitStructure);

	  /* Configure USART Rx as input floating */
	  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	  GPIO_InitStructure.GPIO_Pin = COM_RX_PIN[COM];
	  GPIO_Init(COM_RX_PORT[COM], &GPIO_InitStructure);

	  break;
  default:
	  return;
	  break;
  }

}


/******************* (C) COPYRIGHT 2010 STMicroelectronics *****END OF FILE****/
