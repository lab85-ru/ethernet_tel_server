#ifndef I2C_HARD_H_
#define I2C_HARD_H_

#include <stdint.h>

#define EEPROM_HW_ADDRESS       0xA0   /* E0 = E1 = E2 = 0 */
#define I2C_EE                  I2C2  //interface number
#define ENV_FLASH_ADR           0xA2  // adress A0=A1=A2=0(A0), A0=1(A2)

#define I2C_OK			0
#define I2C_ERROR_RW		1
#define I2C_ERROR_ADDR		2

void I2C_Configuration(void);

uint8_t I2C_F_ByteWrite(uint16_t adr, uint8_t val);
uint8_t I2C_F_ByteRead( uint16_t ReadAddr, uint8_t *val);

#endif