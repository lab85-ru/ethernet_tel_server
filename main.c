//******************************************************************************
// Версия с расширенной I2C памятью для хранения коментариев-описания 
// для портов Ввод/Выводов, память FM24CL64.
//******************************************************************************

#include "stm32f10x_conf.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "board.h"
#include "main.h"
#include "task_consol.h"
#include "task_uip.h"
#include "adc.h"

#include "queue_buf.h"
#include "init_hardware.h"
#include "heap_z.h"

#include "uip.h"
#include "printf_hal.h"
#include "git_commit.h"

const int k1 = 8;        // коэффициенты k1/k2 = 3.3v/4096 = 8/10000
const int k2 = 10000;

const char ERROR_R_ENV[] = {"\r\nERROR: env_read_value\r\n"};
const char ERROR_W_ENV[] = {"\r\nERROR: env_write_value\r\n"};

volatile int table_mu[ ADC_CHANNEL_QUANTITY ];  // таблици коэффициентов множитель и делитель
volatile int table_di[ ADC_CHANNEL_QUANTITY ];

char txt_buf[ TXT_BUF_SIZE ];                   // буфер для чтения - записи строк из-в i2c память (конфигурация устройства)
char html_buf[HTML_BUF_SIZE];                   // буфер для realtime сборки WEB HTTP страници

const char txt_device_ver_soft[]={"SV:1.3.18"};	// версия прошивки софт
const char txt_device_ver_hard[]={"HV:1.0.12"};	// версия прошивки железа
const char txt_device_name[]={"SERVER_DAT"};  	// текстовое описание устройства
struct uip_eth_addr mac_addr;                   // здесь храним мак адрес для ethernet
device_status_t device_status;                  // структура текущее состояние устройства входы-выходы

struct queue_buffer consol_rx;
uint8_t consol_rx_buf[ QUEUE_CONSOL_RX_SIZE ];

unsigned int adc_channel_num = 0;               // номер текущего канала АЦП 
rehim_status adc_conv_status = STOP;            // текущее состояние работы АЦП (умолчанию остановлено)

volatile portBASE_TYPE res;
volatile xSemaphoreHandle xUartConsolOSsem; 	// семафор консоли
volatile xSemaphoreHandle xI2COSsem; 			// семафор I2C шины

xTaskHandle xTaskHandleConsol;
xTaskHandle xTaskHandleUIP;

void tx_byte(uint8_t ch);
void device_status_update_adc(void);
void device_status_update_gpio(void);

////////////////////////////////////////////////////////////////////////////////
// MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN
////////////////////////////////////////////////////////////////////////////////
int main()
{
    xfunc_out = tx_byte;           // настройка указателя на функцию вывода (для xprintf.c)

  	// this sets up the oscilator
	SystemInit();
        
    init_hardware();
    init_system_heap();

    printf_d("\r\n\r\nDevice start.\n");

    if (RCC_GetFlagStatus(RCC_FLAG_IWDGRST) != RESET){
	    /* IWDGRST flag set */
		printf_d("\r\nHard Reset From WatchDog....\r\n");
		/* Clear reset flags */
		RCC_ClearFlag();
	}

    printf_d("================================================================================\r\n");
    printf_d(" Soft Version = %s\r\n", txt_device_ver_soft);
    printf_d(" Hard Version = %s\r\n", txt_device_ver_hard);
    printf_d(" Device Name  = %s\r\n", txt_device_name);
    printf_d(" GIT commit   = %s\r\n", git_commit_str);
    printf_d("================================================================================\r\n");
    printf_d(" Ethernet CHIP driver ENC28J60.\n");
    
	//Create sem
	vSemaphoreCreateBinary(xUartConsolOSsem);
	vSemaphoreCreateBinary(xI2COSsem);

	res = xTaskCreate( vTaskConsol, (const signed portCHAR *)"Consol", config_TASK_Consol_STACK_SIZE, NULL, mainCONSOL_TASK_PRIORITY, &xTaskHandleConsol );
    if (res != pdPASS){
        printf_d("ERROR: create task Consol.\n");
    }

	res = xTaskCreate( vTask_uIP, ( signed char * ) "uIP", configMINIMAL_STACK_SIZE*2, NULL, mainUIP_TASK_PRIORITY, &xTaskHandleUIP);
    if (res != pdPASS){
        printf_d("ERROR: create task uIP.\n");
    }
       
 	/* Start the scheduler. */
	vTaskStartScheduler();

	return 0;
}
//------------------------------------------------------------------------------
// Вывод в консоль
//------------------------------------------------------------------------------
void tx_byte(uint8_t ch)
{
    /* Loop until the end of transmission */
    while (USART_GetFlagStatus(H_COM1, USART_FLAG_TC) == RESET);
    /* Place your implementation of fputc here */
    /* e.g. write a character to the USART */
    USART_SendData(H_COM1, (uint8_t) ch);
}

void vApplicationStackOverflowHook( xTaskHandle *pxTask, signed portCHAR *pcTaskName )
{
	printf_dos("vApplicationStackOverflowHook=%s",pcTaskName);
	while(1);
}

//------------------------------------------------------------------------------
// Захватить семафор печати на консоль 
//------------------------------------------------------------------------------
void ConPrintSemTake( void )
{
    while(xSemaphoreTake(xUartConsolOSsem, portMAX_DELAY) == pdFALSE);
}

//------------------------------------------------------------------------------
// Освободить семафор печати на консоль 
//------------------------------------------------------------------------------
void ConPrintSemGive( void )
{
    xSemaphoreGive(xUartConsolOSsem);
}

//---------------------------------------------------------------------------
// Обновляем данные от АЦП
//---------------------------------------------------------------------------
void device_status_update_adc(void)
{
    if (adc_conv_status == STOP){
        adc_conv_status = START;
        adc_channel_num = 0; 			// запускаем преобразование АЦП начиная с 0 канала
		adc_channel_setup(adc_channel_num);	// переключили на номер канала в начало
        adc_start();
    }
}

//---------------------------------------------------------------------------
// Обновляем данные от GPIO
//---------------------------------------------------------------------------
void device_status_update_gpio(void)
{
    device_status.gpio_input = IN_GPIO_PORT->IDR & IN_PIN_ALL;
    device_status.gpio_output = (OUT_GPIO_PORT->IDR & OUT_PIN_ALL) >> 8;
}

void vApplicationMallocFailedHook( size_t size )
{
	printf_d("!!!!!!!! vApplicationMallocFailedHook size=%d not FOUND\r\n",size);
}

void uip_log(char *msg) 
{
    printf_dos("UIPLOG: %s\n", msg);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////
// hex out
///////////////////////////////////////////////////////////////////////////////////////////////////////////
#define DUMP_STRING_LEN (16)
void hex_out(const char *buf_in, unsigned long len)
{
    unsigned long n,i,j;
    char c;
    char s[DUMP_STRING_LEN];

    printf("\n");
    n=0;

    for (i=0; i<len; i++){
        c = *(buf_in + i);
        printf(" %02X",(unsigned char)c);

        if (c<0x20)
            s[n] = '.';
        else
            s[n] = c;

        n++;
        if (n == DUMP_STRING_LEN){
            printf(" | ");
            for (j=0;j<DUMP_STRING_LEN;j++) 
                printf("%c",s[j]);
                n=0;
                printf("\n");
                //printf("addres= %04lx: ",i+1);
        }
    }

    if (n > 0 && n < DUMP_STRING_LEN){
        for (j=0;j<DUMP_STRING_LEN-n;j++) 
            printf("   ");
        printf(" | ");
        for (j=0;j<n;j++) 
            printf("%c",s[j]);
    }

    printf("\n");
}
