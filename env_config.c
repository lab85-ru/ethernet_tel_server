#include <string.h>

#include "main.h"
#include "env_config.h"
#include "i2c.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "printf_hal.h"

extern char txt_buf[];
extern xSemaphoreHandle xI2COSsem; 			// семафор I2C шины

#ifndef DEBUG
#define DEBUG 0
#endif



const values_st config_env_table[] = {
{ 
    .name = R_SN,
    .size = SN_LEN,
},
{
    .name = R_TXT, 
    .size = TXT_LEN,
},
{
    .name = R_IP_DEVICE, 
    .size = SETIPLAN_LEN,
},
{
    .name = R_IP_GW, 
    .size = SETIPLAN_LEN,
},
{
    .name = R_IP_MASK, 
    .size = SETIPLAN_LEN,
},
{
    .name = R_UDP_PORT, 
    .size = SETPORTLAN_LEN,
},

{
    .name = R_ADC_0, 
    .size = TXT_REM_LEN,
},
{
    .name = R_ADC_1, 
    .size = TXT_REM_LEN,
},
{
    .name = R_ADC_2, 
    .size = TXT_REM_LEN,
},
{
    .name = R_ADC_3, 
    .size = TXT_REM_LEN,
},
{
    .name = R_ADC_4, 
    .size = TXT_REM_LEN,
},
{
    .name = R_ADC_5, 
    .size = TXT_REM_LEN,
},
{
    .name = R_ADC_6, 
    .size = TXT_REM_LEN,
},
{
    .name = R_ADC_7, 
    .size = TXT_REM_LEN,
},

{
    .name = R_INPUT_0, 
    .size = TXT_REM_LEN,
},
{
    .name = R_INPUT_1, 
    .size = TXT_REM_LEN,
},
{
    .name = R_INPUT_2, 
    .size = TXT_REM_LEN,
},
{
    .name = R_INPUT_3, 
    .size = TXT_REM_LEN,
},
{
    .name = R_INPUT_4, 
    .size = TXT_REM_LEN,
},
{
    .name = R_INPUT_5, 
    .size = TXT_REM_LEN,
},
{
    .name = R_INPUT_6, 
    .size = TXT_REM_LEN,
},
{
    .name = R_INPUT_7, 
    .size = TXT_REM_LEN,
},

{
    .name = R_OUTPUT_0, 
    .size = TXT_REM_LEN,
},
{
    .name = R_OUTPUT_1, 
    .size = TXT_REM_LEN,
},
{
    .name = R_OUTPUT_2, 
    .size = TXT_REM_LEN,
},
{
    .name = R_OUTPUT_3, 
    .size = TXT_REM_LEN,
},
{
    .name = R_OUTPUT_4, 
    .size = TXT_REM_LEN,
},
{
    .name = R_OUTPUT_5, 
    .size = TXT_REM_LEN,
},
{
    .name = R_OUTPUT_6, 
    .size = TXT_REM_LEN,
},
{
    .name = R_OUTPUT_7, 
    .size = TXT_REM_LEN,
},

{
    .name = R_MUL_0, 
    .size = TXT_MULDIV_LEN,
},
{
    .name = R_DIV_0, 
    .size = TXT_MULDIV_LEN,
},
{
    .name = R_MUL_1, 
    .size = TXT_MULDIV_LEN,
},
{
    .name = R_DIV_1, 
    .size = TXT_MULDIV_LEN,
},
{
    .name = R_MUL_2, 
    .size = TXT_MULDIV_LEN,
},
{
    .name = R_DIV_2, 
    .size = TXT_MULDIV_LEN,
},
{
    .name = R_MUL_3, 
    .size = TXT_MULDIV_LEN,
},
{
    .name = R_DIV_3, 
    .size = TXT_MULDIV_LEN,
},
{
    .name = R_MUL_4, 
    .size = TXT_MULDIV_LEN,
},
{
    .name = R_DIV_4, 
    .size = TXT_MULDIV_LEN,
},
{
    .name = R_MUL_5, 
    .size = TXT_MULDIV_LEN,
},
{
    .name = R_DIV_5, 
    .size = TXT_MULDIV_LEN,
},
{
    .name = R_MUL_6, 
    .size = TXT_MULDIV_LEN,
},
{
    .name = R_DIV_6, 
    .size = TXT_MULDIV_LEN,
},
{
    .name = R_MUL_7, 
    .size = TXT_MULDIV_LEN,
},
{
    .name = R_DIV_7, 
    .size = TXT_MULDIV_LEN,
},



};

//-----------------------------------------------------------------------------
// поиск в таблице элемента
// возвращает указатель структуру в которой находится длинна и размер записи
// Если вернул !0 значит элемент не найден
//-----------------------------------------------------------------------------
int find_values(uint8_t rec, values_adr_size_st *val)
{
    uint16_t adr = 0;
    int i;

    for(i=0; i<sizeof(config_env_table)/sizeof(values_st); i++){
        if (rec == config_env_table[i].name){
            val->size = config_env_table[i].size;
            val->adr = adr;

            return 0;
        }
        adr = adr + config_env_table[i].size;
    }

    val->size = 0;
    val->adr = 0;

    return 1;
}

void print_env(void)
{

    if(env_read_value(R_SN, txt_buf)){
        printf_dos("ERROR: print_env R_SN\n");
        txt_buf[0] = '\0';
    }
	printf_dos("sn         = %s\n\r", txt_buf);
    
    if(env_read_value(R_TXT, txt_buf)){
        printf_dos("ERROR: print_env R_TXT\n");
        txt_buf[0] = '\0';
    }
	printf_dos("txt        = %s\n\r", txt_buf);

    if(env_read_value(R_IP_DEVICE, txt_buf)){
        printf_dos("ERROR: print_env R_IP_DEVICE\n");
        txt_buf[0] = '\0';
    }
	printf_dos("IP Device  = %s\n\r", txt_buf);

    if(env_read_value(R_IP_GW, txt_buf)){
        printf_dos("ERROR: print_env R_IP_GW\n");
        txt_buf[0] = '\0';
    }
	printf_dos("IP gateway = %s\n\r", txt_buf);

    if(env_read_value(R_IP_MASK, txt_buf)){
        printf_dos("ERROR: print_env R_IP_MASK\n");
        txt_buf[0] = '\0';
    }
	printf_dos("IP MASK    = %s\n\r", txt_buf);

}

void var_clr(unsigned char *buf, unsigned char len)
{
	if ( *buf == 0xff) memset( buf, 0, len );
}

//-----------------------------------------------------------------------------
// чтение СТРОКИ значения переменной из массива
//-----------------------------------------------------------------------------
unsigned char env_read_value(const uint8_t rec, char *buf)
{
    uint8_t res;
    values_adr_size_st val;

    if (find_values(rec, &val)){ // ошибка запись не найдена
        printf_d("ERROR: env_read_value find_values...\n");
        *buf = '\0';
        return 1;
    }

    memset(buf, 0, val.size);

//    b[0] = (val.adr & 0xff00)>> 8;
//    b[1] = val.adr & 0xff;
//    res = I2C_paketWrite( ENV_FLASH_ADR, b, 2); // выставляем адрес
//    if (res != I2C_OK){
//        printf_d("ERROR: env_read_value I2C_paketWrite return result=%d\n", res);
//        *buf = '\0';
//        return res;
//    }

//    res = I2C_paketRead( ENV_FLASH_ADR, (uint8_t*)buf, val.size);
//    if (res != I2C_OK){
//        printf_d("ERROR: env_read_value I2C_paketRead return result=%d\n", res);
//        *buf = '\0';
//        return res;
//    }

    for (uint16_t i=0; i<val.size; i++){
        while(xSemaphoreTake(xI2COSsem, portMAX_DELAY) == pdFALSE);
        res = I2C_F_ByteRead(val.adr + i, (uint8_t*)&buf[i]);
        xSemaphoreGive(xI2COSsem);
        if (res){
            printf_d("ERROR: env_read_value I2C_F_ByteRead result = %d\n", res);
        }
    }
    
    if (strlen(buf) >= val.size){
        printf_d("WARNING: env_read_value strlen(buf) > val_len_size\n");
        *(buf + val.size - 1) = '\0'; // усекаем выходную строку(чтобы когда будет читать, не читать больше чем длинна максимальной строки)
    }

    res = 0;
    
    return res;
}

//-----------------------------------------------------------------------------
// Запись СТРОКИ в массив
//-----------------------------------------------------------------------------
uint8_t env_write_value(const uint8_t rec, char *buf)
{
    uint8_t res;
    uint16_t l;
    uint16_t adr = 0;
    values_adr_size_st val;
    uint16_t i;

    if (find_values(rec, &val)){ // ошибка запись не найдена
        printf_d("ERROR: env_write_value find_values...\n");
        return 1;
    }

    l = strlen(buf);
    if (l >= val.size){
        l = val.size - 1; // ограничивае длинну + запас на \0
        buf[l] = 0;
    }

    adr = val.adr;
    for (i = 0; i <= l; i++){// внимание i <= l т.к. в конце записываем \0 !!!
        while(xSemaphoreTake(xI2COSsem, portMAX_DELAY) == pdFALSE);
        res = I2C_F_ByteWrite(adr + i, buf[i]);
        xSemaphoreGive(xI2COSsem);
        if (res){
            printf_d("ERROR: env_write_values I2C_paketWrite return result=%d\n", res);
            return res;
        }
        //buf++;
    }// for

    return res;
}

