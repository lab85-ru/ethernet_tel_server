//***************************************************************************//**
//EEPROM LIB v 1.0
//******************************************************************************/
#include "stm32f10x_gpio.h"
#include "stm32f10x_i2c.h"
#include "stm32f10x_rcc.h"

#include <stdint.h>
//#include "FreeRTOS.h"
//#include "task.h"
#include "printf_hal.h"
#include "i2c.h"


#define I2C_TIMEOUT (1000000)  // таймаут на выполнение операции ожидания, это счетчик.

#define TIME_OUT_COUNT() \
    { c++; \
    if (c == I2C_TIMEOUT){ \
        printf_d("ERROR: IO I2C_TIMEOUT.\n"); \
        return 1; \
    } \
    }


/***************************************************************************//**
 *  @brief  I2C Configuration
 ******************************************************************************/
void I2C_Configuration(void)
{

   	   I2C_InitTypeDef  I2C_InitStructure;
	   GPIO_InitTypeDef  GPIO_InitStructure;

	   RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2,ENABLE);

	   RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB| RCC_APB2Periph_AFIO , ENABLE);//

	   /* Configure I2C1 pins: PB6->SCL and PB7->SDA */
	   /* Configure I2C2 pins: PB10->SCL and PB11->SDA */
           
	   GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_10 | GPIO_Pin_11;
	   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
	   GPIO_Init(GPIOB, &GPIO_InitStructure);

	   I2C_DeInit(I2C_EE);
	   I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
	   I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_16_9;
	   I2C_InitStructure.I2C_OwnAddress1 = 1;
	   I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
	   I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	   I2C_InitStructure.I2C_ClockSpeed = 100000;  /* 100kHz */

	   I2C_Cmd(I2C_EE, ENABLE);
	   I2C_Init(I2C_EE, &I2C_InitStructure);
	   I2C_AcknowledgeConfig(I2C_EE, ENABLE);

}

//*******************************************************************8
//***************************************************************

uint8_t I2C_F_ByteWrite(uint16_t adr, uint8_t val)
{
    uint32_t c;
    static uint8_t adr_hi;// = (uint8_t)((adr & 0xFF00) >> 8);
    static uint8_t adr_lo;// = (uint8_t)(adr & 0xFF);
    
    adr_hi = (uint8_t)((adr & 0xFF00) >> 8);
    adr_lo = (uint8_t)(adr & 0xFF);
    
    /* While the bus is busy */
    c = 0;
    while(I2C_GetFlagStatus(I2C_EE, I2C_FLAG_BUSY)){
        TIME_OUT_COUNT();
    }
    
    /* Send START condition */
    I2C_GenerateSTART(I2C_EE, ENABLE);

    /* Test on EV5 and clear it */
    while(!I2C_CheckEvent(I2C_EE, I2C_EVENT_MASTER_MODE_SELECT)){
        TIME_OUT_COUNT();
    }

    /* Send EEPROM address for write */
    I2C_Send7bitAddress(I2C_EE, ENV_FLASH_ADR, I2C_Direction_Transmitter);

    /* Test on EV6 and clear it */
    while(!I2C_CheckEvent(I2C_EE, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)){
        TIME_OUT_COUNT();
    }

    /* Send the EEPROM's internal address to write to : MSB of the address first */
    I2C_SendData(I2C_EE, adr_hi);

    /* Test on EV8 and clear it */
    while(!I2C_CheckEvent(I2C_EE, I2C_EVENT_MASTER_BYTE_TRANSMITTED)){
        TIME_OUT_COUNT();
    }

    /* Send the EEPROM's internal address to write to : LSB of the address */
    I2C_SendData(I2C_EE, adr_lo);

    /* Test on EV8 and clear it */
    while(! I2C_CheckEvent(I2C_EE, I2C_EVENT_MASTER_BYTE_TRANSMITTED)){
        TIME_OUT_COUNT();
    }

     I2C_SendData(I2C_EE, val);

    /* Test on EV8 and clear it */
    while (!I2C_CheckEvent(I2C_EE, I2C_EVENT_MASTER_BYTE_TRANSMITTED)){
        TIME_OUT_COUNT();
    }

    /* Send STOP condition */
    I2C_GenerateSTOP(I2C_EE, ENABLE);

    return 0;
}
//*********************************************************************************
uint8_t I2C_F_ByteRead( uint16_t ReadAddr, uint8_t *val)
{
    uint8_t tmp;
    uint32_t c;

	/* While the bus is busy */
    c = 0;
    while(I2C_GetFlagStatus(I2C_EE, I2C_FLAG_BUSY)){
        TIME_OUT_COUNT();
    }

    /* Send START condition */
    I2C_GenerateSTART(I2C_EE, ENABLE);

    /* Test on EV5 and clear it */
    c = 0;
    while(!I2C_CheckEvent(I2C_EE, I2C_EVENT_MASTER_MODE_SELECT)){
        TIME_OUT_COUNT();
    }

    /* Send EEPROM address for write */
    I2C_Send7bitAddress(I2C_EE, ENV_FLASH_ADR, I2C_Direction_Transmitter);

    /* Test on EV6 and clear it */
    c = 0;
    while(!I2C_CheckEvent(I2C_EE, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)){
        TIME_OUT_COUNT();
    }

    /* Send the EEPROM's internal address to read from: MSB of the address first */
    I2C_SendData(I2C_EE, (uint8_t)((ReadAddr & 0xFF00) >> 8));

    /* Test on EV8 and clear it */
    while(!I2C_CheckEvent(I2C_EE, I2C_EVENT_MASTER_BYTE_TRANSMITTED)){
        TIME_OUT_COUNT();
    }

    /* Send the EEPROM's internal address to read from: LSB of the address */
    I2C_SendData(I2C_EE, (uint8_t)(ReadAddr & 0x00FF));

    /* Test on EV8 and clear it */
    c = 0;
    while(!I2C_CheckEvent(I2C_EE, I2C_EVENT_MASTER_BYTE_TRANSMITTED)){
        TIME_OUT_COUNT();
    }

    /* Send STRAT condition a second time */
    I2C_GenerateSTART(I2C_EE, ENABLE);

    /* Test on EV5 and clear it */
    c = 0;
    while(!I2C_CheckEvent(I2C_EE, I2C_EVENT_MASTER_MODE_SELECT)){
        TIME_OUT_COUNT();
    }

    /* Send EEPROM address for read */
    I2C_Send7bitAddress(I2C_EE, ENV_FLASH_ADR, I2C_Direction_Receiver);

    /* Test on EV6 and clear it */
    c = 0;
    while (!I2C_CheckEvent(I2C_EE,I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED)){ // Wait for EV6
        TIME_OUT_COUNT();
    }

    c = 0;
	while (!I2C_CheckEvent(I2C_EE,I2C_EVENT_MASTER_BYTE_RECEIVED)){ // Wait for EV7 (Byte received from slave)
        TIME_OUT_COUNT();
    }
    
    tmp = I2C_ReceiveData(I2C_EE);

    if (val != 0 ){
        *val = tmp;
    }

    I2C_AcknowledgeConfig(I2C_EE, DISABLE);

    /* Send STOP Condition */
    I2C_GenerateSTOP(I2C_EE, ENABLE);
    
    return 0;
}
//*******************************************************************************
