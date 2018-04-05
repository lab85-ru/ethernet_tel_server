/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BOARD_H
#define __BOARD_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

#define LEDn	2

#define LEDR_PIN                         GPIO_Pin_4
#define LEDR_GPIO_PORT                   GPIOA
#define LEDR_GPIO_CLK                    RCC_APB2Periph_GPIOA

#define LEDG_PIN                         GPIO_Pin_5
#define LEDG_GPIO_PORT                   GPIOA
#define LEDG_GPIO_CLK                    RCC_APB2Periph_GPIOA

// line int for enc28j60
#define ETH_INT_PIN                GPIO_Pin_8
#define ETH_INT_GPIO_PORT          GPIOA
#define ETH_INT_GPIO_CLK           RCC_APB2Periph_GPIOA
#define ETH_INT_EXTI_LINE          EXTI_Line8
#define ETH_INT_EXTI_PORT_SOURCE   GPIO_PortSourceGPIOA
#define ETH_INT_EXTI_PIN_SOURCE    GPIO_PinSource8
#define ETH_INT_EXTI_IRQn          EXTI9_5_IRQn

// ¬нешние входные порты
#define IN_PIN_0                         GPIO_Pin_0
#define IN_PIN_1                         GPIO_Pin_1
#define IN_PIN_2                         GPIO_Pin_2
#define IN_PIN_3                         GPIO_Pin_3
#define IN_PIN_4                         GPIO_Pin_4
#define IN_PIN_5                         GPIO_Pin_5
#define IN_PIN_6                         GPIO_Pin_6
#define IN_PIN_7                         GPIO_Pin_7

#define IN_PIN_ALL                 (IN_PIN_0 | IN_PIN_1 | IN_PIN_2 | IN_PIN_3 | IN_PIN_4 | IN_PIN_5 | IN_PIN_6 | IN_PIN_7)
#define IN_GPIO_PORT               GPIOC
#define IN_GPIO_CLK                RCC_APB2Periph_GPIOC

// ¬нешние вџходные порты
#define OUT_PIN_0                         GPIO_Pin_8
#define OUT_PIN_1                         GPIO_Pin_9
#define OUT_PIN_2                         GPIO_Pin_10
#define OUT_PIN_3                         GPIO_Pin_11
#define OUT_PIN_4                         GPIO_Pin_12
#define OUT_PIN_5                         GPIO_Pin_13
#define OUT_PIN_6                         GPIO_Pin_14
#define OUT_PIN_7                         GPIO_Pin_15

#define OUT_PIN_ALL                 (OUT_PIN_0 | OUT_PIN_1 | OUT_PIN_2 | OUT_PIN_3 | OUT_PIN_4 | OUT_PIN_5 | OUT_PIN_6 | OUT_PIN_7)
#define OUT_GPIO_PORT                   GPIOC
#define OUT_GPIO_CLK                    RCC_APB2Periph_GPIOC
   

// ¬нешние входные порты ADC
#define ADC_Channel                  (ADC_Channel_0 | ADC_Channel_1 | ADC_Channel_2 | ADC_Channel_3 | ADC_Channel_4 | ADC_Channel_5 | ADC_Channel_6 | ADC_Channel_7)   
#define ADC_PIN_0                         GPIO_Pin_0
#define ADC_PIN_1                         GPIO_Pin_1
#define ADC_PIN_2                         GPIO_Pin_2
#define ADC_PIN_3                         GPIO_Pin_3
#define ADC_PIN_4                         GPIO_Pin_4
#define ADC_PIN_5                         GPIO_Pin_5
#define ADC_PIN_6                         GPIO_Pin_6
#define ADC_PIN_7                         GPIO_Pin_7

#define ADC_GPIO_PORT                   GPIOA
#define ADC_GPIO_CLK                    RCC_APB2Periph_GPIOA
   
/**
  * @}
  */ 
  
/** @addtogroup STM32100B_EVAL_LOW_LEVEL_BUTTON
  * @{
  */  
 typedef enum
 {
   LED_R = 0,
   LED_G = 1
 } Led_TypeDef;


 typedef enum
 {
   COM1 = 0,
   COM2 = 1,
   COM3 = 2,
//   COM4 = 3,
//   COM5 = 4
 } COM_TypeDef;


/**
  * @}
  */ 

/** @addtogroup STM32100B_EVAL_LOW_LEVEL_COM
  * @{
  */
#define COMn                             3


/**
 * @brief Definition for COM port1, connected to USART1
 * RING-Consol
 */ 
#define H_COM1                        USART1
#define H_COM1_CLK                    RCC_APB2Periph_USART1
#define H_COM1_TX_PIN                 GPIO_Pin_9
#define H_COM1_TX_GPIO_PORT           GPIOA
#define H_COM1_TX_GPIO_CLK            RCC_APB2Periph_GPIOA
#define H_COM1_RX_PIN                 GPIO_Pin_10
#define H_COM1_RX_GPIO_PORT           GPIOA
#define H_COM1_RX_GPIO_CLK            RCC_APB2Periph_GPIOA
#define H_COM1_IRQn                   USART1_IRQn

/**
 * @brief Definition for COM port2, connected to USART2
 */ 
#define H_COM2                        USART2
#define H_COM2_CLK                    RCC_APB1Periph_USART2
#define H_COM2_TX_PIN                 GPIO_Pin_2
#define H_COM2_TX_GPIO_PORT           GPIOA
#define H_COM2_TX_GPIO_CLK            RCC_APB2Periph_GPIOA
#define H_COM2_RX_PIN                 GPIO_Pin_3
#define H_COM2_RX_GPIO_PORT           GPIOA
#define H_COM2_RX_GPIO_CLK            RCC_APB2Periph_GPIOA
#define H_COM2_IRQn                   USART2_IRQn

/**
 * @brief Definition for COM port3, connected to USART3
 */
#define H_COM3                        USART3
#define H_COM3_CLK                    RCC_APB1Periph_USART3
#define H_COM3_TX_PIN                 GPIO_Pin_10
#define H_COM3_TX_GPIO_PORT           GPIOB
#define H_COM3_TX_GPIO_CLK            RCC_APB2Periph_GPIOB
#define H_COM3_RX_PIN                 GPIO_Pin_11
#define H_COM3_RX_GPIO_PORT           GPIOB
#define H_COM3_RX_GPIO_CLK            RCC_APB2Periph_GPIOB
#define H_COM3_IRQn                   USART3_IRQn

/**
 * @brief Definition for COM port4, connected to UART4
 */
#define H_COM4                        UART4
#define H_COM4_CLK                    RCC_APB1Periph_UART4
#define H_COM4_TX_PIN                 GPIO_Pin_11
#define H_COM4_TX_GPIO_PORT           GPIOC
#define H_COM4_TX_GPIO_CLK            RCC_APB2Periph_GPIOC
#define H_COM4_RX_PIN                 GPIO_Pin_10
#define H_COM4_RX_GPIO_PORT           GPIOC
#define H_COM4_RX_GPIO_CLK            RCC_APB2Periph_GPIOC
#define H_COM4_IRQn                   USART4_IRQn

/**
 * @brief Definition for COM port5, connected to UART5
 */
#define H_COM5                        UART5
#define H_COM5_CLK                    RCC_APB1Periph_UART5
#define H_COM5_TX_PIN                 GPIO_Pin_12
#define H_COM5_TX_GPIO_PORT           GPIOC
#define H_COM5_TX_GPIO_CLK            RCC_APB2Periph_GPIOC
#define H_COM5_RX_PIN                 GPIO_Pin_2
#define H_COM5_RX_GPIO_PORT           GPIOD
#define H_COM5_RX_GPIO_CLK            RCC_APB2Periph_GPIOD
#define H_COM5_IRQn                   USART5_IRQn




void com_init(uint8_t port_num, uint32_t baud);
void com_tx_start( uint8_t port_num );
void nvic_config_com1(FunctionalState st);
void nvic_config_com2(FunctionalState st);
void nvic_config_com3(FunctionalState st);
//void stm32_com_init(COM_TypeDef COM, USART_InitTypeDef* USART_InitStruct);
void stm32_uart_set(COM_TypeDef COM, USART_InitTypeDef* USART_InitStruct, FunctionalState st);
//void stm32_uart_init(COM_TypeDef COM, USART_InitTypeDef* USART_InitStruct);
void stm32_uart_init(COM_TypeDef COM);

void STM_EVAL_LEDInit(Led_TypeDef Led);
void STM_EVAL_LEDOn(Led_TypeDef Led);
void STM_EVAL_LEDOff(Led_TypeDef Led);
void STM_EVAL_LEDToggle(Led_TypeDef Led);
void STM_EVAL_COMInit(COM_TypeDef COM, USART_InitTypeDef* USART_InitStruct);


/**
  * @}
  */ 
    
#ifdef __cplusplus
}
#endif
  
#endif /* __BOARD_H */
