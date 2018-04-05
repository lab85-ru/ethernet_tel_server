/**
  ******************************************************************************
  * @file    Project/STM32F10x_StdPeriph_Template/stm32f10x_it.c 
  * @author  MCD Application Team
  * @version V3.5.0
  * @date    08-April-2011
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_it.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "semphr.h"

#include "queue.h"
#include "board.h"
#include "main.h"
#include "adc.h"

#include "queue_buf.h"
/** @addtogroup STM32F10x_StdPeriph_Template
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
extern struct queue_buffer consol_rx;
extern xTaskHandle xTaskHandleConsol;
extern xTaskHandle xTaskHandleUIP;
extern device_status_t device_status;
extern unsigned int adc_channel_num;
extern rehim_status adc_conv_status;
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
	vPortSVCHandler();
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
	xPortPendSVHandler();
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
	xPortSysTickHandler();
}

/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_xx.s).                                            */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/
void USART1_IRQHandler(void)// consol
{
	uint8_t c;

	if(USART_GetITStatus(H_COM1, USART_IT_RXNE) != RESET){
		c = (USART_ReceiveData(H_COM1) & 0xFF);
		if ( free_queue( &consol_rx ) > 0 )
			push_data_queue_b( &consol_rx ,c );
		//xTaskResumeFromISR(xTaskHandleConsol);
	}

	if(USART_GetITStatus(H_COM1, USART_IT_TXE) != RESET){   
		USART_ITConfig(H_COM1, USART_IT_TXE, DISABLE);
	}
}


// int in ethernet

//void EXTI9_5_IRQHandler(void)
//{
//        if(EXTI_GetITStatus(ETH_INT_EXTI_LINE) != RESET){
//                xTaskResumeFromISR(xTaskHandleUIP);
//         	EXTI_ClearITPendingBit(ETH_INT_EXTI_LINE);/* Clear the EXTI line pending bit */
//	}
//}


void ADC1_IRQHandler(void)
{
        device_status.adc_converted_value[adc_channel_num] = ADC_GetConversionValue(ADC1);
        ADC_ClearFlag(ADC1, ADC_FLAG_EOC);                                                // сбросили флаг прерывания
        if (adc_channel_num < ADC_CHANNEL_QUANTITY){
                adc_channel_num++;
                //ADC_RegularChannelConfig(ADC1, adc_channel_num, 1, ADC_SampleTime_55Cycles5);   // переключаем АЦП на другой канал (adc_channel_num диапазон значений [0-7])
                adc_channel_setup(adc_channel_num);
                //ADC_SoftwareStartConvCmd(ADC1, ENABLE);/* Start ADC1 Software Conversion */
                adc_start();
        }else adc_conv_status = STOP;
}

/**
  * @}
  */ 


/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
