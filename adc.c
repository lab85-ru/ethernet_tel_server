//==============================================================================
// ADC HAL for stm32
//==============================================================================
#include <stdint.h>
#include "stm32f10x.h"
#include "stm32f10x_conf.h"

#include "board.h"

//------------------------------------------------------------------------------
// Подключение АЦП к каналу с номером ADC_Channel [диапазон значений в данном устройстве 0-7]
//------------------------------------------------------------------------------
void adc_channel_setup(uint8_t adc_channel)
{
        /* ADC1 regular channel0 configuration */ 
        // Начинаем преобразование с 0 канала и до 7(переключения на дргугой канал производится в прерывании после окончания преобразования АЦП)
        ADC_RegularChannelConfig(ADC1, adc_channel, 1, ADC_SampleTime_55Cycles5);
}


//------------------------------------------------------------------------------
// настройка АЦП
//------------------------------------------------------------------------------
void adc_init(void)
{
        ADC_InitTypeDef ADC_InitStructure;
        GPIO_InitTypeDef  GPIO_InitStructure;
        
       	// switch clock to GPIO_ADC_INPUT
	RCC_APB2PeriphClockCmd( ADC_GPIO_CLK, ENABLE );

	GPIO_InitStructure.GPIO_Pin = ADC_PIN_0 | ADC_PIN_1 | ADC_PIN_2 | ADC_PIN_3 | ADC_PIN_4 | ADC_PIN_5 | ADC_PIN_6 | ADC_PIN_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init( ADC_GPIO_PORT, &GPIO_InitStructure );
        
        // Подал тактовые импульсы на АЦП.
        RCC_ADCCLKConfig(RCC_PCLK2_Div6);
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

        /* ADC1 configuration ------------------------------------------------------*/
        ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
        ADC_InitStructure.ADC_ScanConvMode = DISABLE;
        ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
        ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
        ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
        ADC_InitStructure.ADC_NbrOfChannel = 1;
        ADC_Init(ADC1, &ADC_InitStructure);

        /* ADC1 regular channel0 configuration */ 
        // Начинаем преобразование с 0 канала и до 7(переключения на дргугой канал производится в прерывании после окончания преобразования АЦП)
        adc_channel_setup(ADC_Channel_0);
  
        /* Enable ADC1 */
        ADC_Cmd(ADC1, ENABLE);
        ADC_ITConfig(ADC1, ADC_IT_EOC, ENABLE);

        // Создал структуру NVIC и заполнил ее значениями
        // Название константы ADC1_IRQn взял из stm32f10x.h
        NVIC_InitTypeDef NVIC_InitStructure;
        NVIC_InitStructure.NVIC_IRQChannel = ADC1_IRQn;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init(&NVIC_InitStructure);
        
        /* Enable ADC1 reset calibaration register */   
        ADC_ResetCalibration(ADC1);
        /* Check the end of ADC1 reset calibration register */
        while(ADC_GetResetCalibrationStatus(ADC1));

        /* Start ADC1 calibaration */
        ADC_StartCalibration(ADC1);
        /* Check the end of ADC1 calibration */
        while(ADC_GetCalibrationStatus(ADC1));
        // тепрь АЦП готово к работе
}

//------------------------------------------------------------------------------
// Запуск АЦП на преобразование (текущего канала)
//------------------------------------------------------------------------------
void adc_start(void)
{
        /* Start ADC1 Software Conversion */ 
        ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}
