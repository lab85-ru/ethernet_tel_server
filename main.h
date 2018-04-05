/*
 * main.h
 *
 */

#ifndef MAIN_H_
#define MAIN_H_

#include <stdint.h>

//ds2482 temp-int
#define U21_ADDR  					(0x18<<1)

//ds2482 temp-ext
#define U17_ADDR  					(0x19)

#define I2C_ADDR_MAC     (0xA0)   /* E0 = E1 = E2 = 0,  24AA02EE48 */
#define I2C_ADDR_FLASH   (0xA2)   /* E0 = E1 = E2 = 0,  FM24C64 8kb FRAM */

#define I_CHANNEL_QUANTITY                              (8) // ���������� ������� Input
#define O_CHANNEL_QUANTITY                              (8) // ���������� ������� Output

#define ADC_CHANNEL_QUANTITY                            (8) // ���������� ������� ���
#define ADC1_DR_Address                                 ((uint32_t)0x4001244C)

extern const char txt_device_ver_soft[];
extern const char txt_device_ver_hard[];
extern const char txt_device_name[];

#define DEFAULT_UDP_PORT_DEF 10000 // UDP ���� �� ���������

#define QUEUE_CONSOL_RX_SIZE	(64)// ������ �������� ������ �������


#define mainLED_TASK_PRIORITY			( tskIDLE_PRIORITY + 1 )
#define mainCONSOL_TASK_PRIORITY		( tskIDLE_PRIORITY + 2 )
#define mainUIP_PERIODIC_TASK_PRIORITY		( tskIDLE_PRIORITY + 2 )
#define mainUIP_TASK_PRIORITY		        ( tskIDLE_PRIORITY + 2 )


#ifndef DEBUG
#define config_TASK_Consol_STACK_SIZE 		(256+30)//configMINIMAL_STACK_SIZE * 2)
#define config_TASK_LedBlink_STACK_SIZE         (128+30)//configMINIMAL_STACK_SIZE * 2)
#else
#define config_TASK_Consol_STACK_SIZE 		(256+30)//configMINIMAL_STACK_SIZE * 2)
#define config_TASK_LedBlink_STACK_SIZE         (128)//configMINIMAL_STACK_SIZE * 2)
#endif

#define ADC_BUF_SIZE        (8)                 // ���������� ��� ������� ��������������

#define UDP_PAKET_VERSION_H (1)                 // ������ UDP ������. ��� ����������� ��������� � ���������� ������.
#define UDP_PAKET_VERSION_L (0)

typedef struct {
        volatile uint8_t gpio_input;
        volatile uint8_t gpio_output;
        volatile uint16_t adc_converted_value[ ADC_CHANNEL_QUANTITY ];   // ������ ��� �������� �������� ���������� �� ���
} device_status_t;

typedef enum { STOP, START } rehim_status;                       // ����������� ���� ��������� ���������

#define CLOCK_SECOND_S            (1000)                         // ��������� ������ ���������� 1000 ��� � ������� FreeRTOS
#define PERIODIC_TIMER_INTERVAL   (CLOCK_SECOND_S / 2)
#define ARP_TIMER_INTERVAL        (CLOCK_SECOND_S * 10)

#define HTML_BUF_SIZE  (500)                    // ������ ������ � ������� ���������� ����� html ������
#define TXT_BUF_SIZE   (60)                     // ������ ������ ������������ ��� I2C ������
#define UDP_PAKET_SIZE (128)                    // ������ UDP ������
#define TRACE_BUF_SIZE (512)                    // ������ ������ ��� ������ ������������� ���������� (������ ���������� �� ����)
#define BUF_P_SIZE     (128)                    // ������ ������ ��� ������ ���������� �� ���, ��� ������ � �������

void device_status_update_adc(void);
void device_status_update_gpio(void);
void hex_out(const char *buf_in, unsigned long len);

#endif /* MAIN_H_ */
