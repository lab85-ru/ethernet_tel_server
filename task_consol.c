/*
 * task_consol.c
 *
 */

#include <string.h>
#include <stdlib.h>
#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "stm32f10x_iwdg.h"
#include "main.h"
#include "consol.h"
#include "queue_buf.h"
#include "env_config.h"
#include "heap_z.h"
#include "uip.h"
#include "i2c.h"
#include "printf_hal.h"
#include "task_uip.h"
#include "uptime.h"
#include "time_hal.h"

extern int table_mu[];
extern int table_di[];
extern const int k1;
extern const int k2;
extern struct uip_eth_addr mac_addr;
extern device_status_t device_status;
extern void task_list( void );
extern struct queue_buffer consol_rx;
extern const char *git_commit_str;

int ip_test(const char * s);
void print_byte_to_bit(uint8_t b);


#define CONSOL_BUF_SIZE 	(50)
char consol_buf[ CONSOL_BUF_SIZE ];
volatile consol_state_e consol_state;


////////////////////////////////////////////////////////////////////////////////
// обработка даннных пришедших с консоли
////////////////////////////////////////////////////////////////////////////////
void vTaskConsol( void *pvParameters )
{
    unsigned char consol_buf_n = 0;
    char *adr = 0;
    uint32_t step = 0;
	char c;
	int i;
    uint8_t rec;      // номер записи в структуре данных env_config
    uptime_data_t ut; // время работы системы


	consol_state = OUT_PROMT;
	reset_queue( &consol_rx );
	step++;//out promt

	while(1){
		step = 0;

		switch(consol_state){
		case GET_CHAR:
			if ( datalen_queue( &consol_rx ) == 0 ){
				vTaskDelay(200);
				step = 0;
				break;
			}
			pop_data_queue_b( &consol_rx , (unsigned char *)&c );
			if ( ((c == KEY_ENTER) || (c == KEY_BACKSPACE)) && (consol_buf_n == 0) ){ // пустая строка
				step++;
				consol_state = OUT_PROMT;
				break;
			}
			if ( c == KEY_BACKSPACE ){
				consol_buf[ consol_buf_n ] = '\0';
				printf_dos("\r%s",promt);
				i = consol_buf_n;
				while(i > 0){
					printf_dos(" ");
					i--;
				}
				printf_dos("\r");
				consol_buf_n--;
				consol_buf[ consol_buf_n ] = '\0';
				printf_dos("%s%s",promt,consol_buf);
				break;
			}
			if ( c == KEY_ENTER ){
				consol_buf[ consol_buf_n ] = '\0';
				adr = strchr(consol_buf, '=');
				if (adr == NULL)
					consol_state = CMD_ANALYS_LOW;
				else
					consol_state = CMD_ANALYS_SET;
				step++;
				if (c == 0x0d)
					printf_dos("\r\n");	// делаем перевод строки чтоб ответы шли начиная с новой строки
				break;
			}

			//putchar(c);// no sem ?!
			printf_dos("%c",c);			// vivodim otvetniy simvol

			if (consol_buf_n < CONSOL_BUF_SIZE ){ 	// добавление символа в буфер
				consol_buf[ consol_buf_n ] = c;
				consol_buf_n++;
			}else{
				consol_state = OUT_ERROR;
				step++;
				break;
			}
			break;

		case OUT_PROMT:
			consol_state = GET_CHAR;
			consol_buf_n = 0;
			printf_dos("\n\r%s",promt);
			break;

		case OUT_ERROR:
			consol_buf_n = 0;
			printf_dos("%s",error);
			consol_state = OUT_PROMT;
			step++;
			break;
		case CMD_ANALYS_LOW:
			if (strcmp(consol_buf, cmd_help1)== 0 || strcmp(consol_buf, cmd_help2)==0){
				printf_dos("%s",txt_help);
				consol_state = OUT_PROMT;
				step++;
				break;
			}
			if (strcmp(consol_buf, cmd_view)== 0){
                				printf_dos("================================================================================\n");
                                uptime(time_get_sec_counter(), &ut);
                                printf_dos("System uptime: %02d Years, %03d Days, %02d:%02d:%02d\n", ut.years, ut.days, ut.hours, ut.min, ut.sec );
				                printf_dos("================================================================================\n");
                                printf_dos("MAC        : ");
                                for(i=0;i<6;i++){
                                    printf_dos("%02X%c", mac_addr.addr[i], i==5 ? ' ' : '-');
                                }
                                printf_dos("\n");

                                print_env();

                                printf_dos("---------------GPIO IN------------------------------------------------\n\r");
                                printf_dos("INPUT = 0x%02x\n\r",device_status.gpio_input);
                                printf_dos("7 6 5 4 3 2 1 0\n\r");
                                print_byte_to_bit(device_status.gpio_input);
                                printf_dos("\n\r---------------GPIO OUT-----------------------------------------------\n\r");
                                printf_dos("OUTPUT = 0x%02x\n\r",device_status.gpio_output);
                                printf_dos("7 6 5 4 3 2 1 0\n\r");
                                print_byte_to_bit(device_status.gpio_output);
                                printf_dos("\n\r---------------ADC 12 bit---------------------------------------------\n\r");
                                printf_dos("| ADC code:   | U adc input:        | K= mul/div | Uinput = Uadc * K |\n\r");
                                printf_dos("+-------------+---------------------+------------+-------------------+\n\r");
                                char *buf_p = NULL;
                                buf_p = (char*) pvPortMalloc( BUF_P_SIZE );
                                if (buf_p != NULL){
                                    for(i=0;i<ADC_CHANNEL_QUANTITY;i++){
                                        calc_volt_rdiv(buf_p, device_status.adc_converted_value[i], k1, k2, 1, 1);
                                        printf_dos("| ADC(0x%04X) | U adc(%d) = %6s v | ", device_status.adc_converted_value[i], i, buf_p);
                                        calc_volt_rdiv(buf_p, 1, 1, 1, table_mu[i], table_di[i]);
                                        printf_dos("K = %6s | ", buf_p);
                                        calc_volt_rdiv(buf_p, device_status.adc_converted_value[i], k1, k2, table_mu[i], table_di[i]);
                                        printf_dos("U rdiv = %6s v |\n", buf_p);
                                    }//for
                                    vPortFree( buf_p );
                                }else printf_dos("ERROR: task_list: Malloc.\r\n");
                                
                                printf_dos("----------------------------------------------------------------------\n\r");

				step++;
				consol_state = OUT_PROMT;
				break;
			}
			if (strcmp(consol_buf, cmd_version)== 0 ){
				printf_dos("%s\n\r",txt_device_name);
				printf_dos("%s\n\r",txt_device_ver_soft);
				printf_dos("%s\n\r",txt_device_ver_hard);
				printf_dos("GIT version = %s\r\n",git_commit_str);
				step++;
				consol_state = OUT_PROMT;
				break;
			}
			if (strcmp(consol_buf, cmd_root)== 0 ){
				printf_dos("%s\n\r",txt_root_help);
				step++;
				consol_state = OUT_PROMT;
				break;
			}
			if (strcmp(consol_buf, cmd_reboot)== 0 ){
				printf_dos("\n\rReboot CPU...\n\r");
                                portDISABLE_INTERRUPTS();
                                SysTick->CTRL  = 0;// остановили системный таймер теперь должен сработать вачдог
				while(1);

//				step++;
//				consol_state = OUT_PROMT;
				break;
			}

			// команда не найдена
			step++;
			consol_state = OUT_ERROR;
			break;

		case CMD_ANALYS_SET:
			// за место = в буфер пишим конец строки, чтоб можно было полноценно сравнить строки
			consol_buf[ adr - consol_buf ] = '\0';
			adr++; //указывает на после =

			if (strcmp(consol_buf, cmd_sn)== 0){
				if( (strlen(adr) != SN_LEN) && (strcspn(adr, STR_CIFRA) != 0) ){
					step++;
					consol_state = OUT_ERROR;
					break;
				}
				step++;
				rec = R_SN;//flash_w_adr = (char*)&device_config.sn;
				consol_state = WRITE_CFG;
				break;
			}
			if (strcmp(consol_buf, cmd_txt)== 0){
				if ( strlen(adr) < TXT_LEN ){
                    rec = R_TXT;
					consol_state = WRITE_CFG;
				}else consol_state = OUT_ERROR;
				step++;
				break;
			}
       			if (strcmp(consol_buf, cmd_ipdevice)== 0){
				if ( strlen(adr) < SETIPLAN_LEN && ip_test(adr) == 0 ){
                    rec = R_IP_DEVICE;
					consol_state = WRITE_CFG;
				}else consol_state = OUT_ERROR;
				step++;
				break;
			}
       			if (strcmp(consol_buf, cmd_ipgw)== 0){
				if ( strlen(adr) < SETIPLAN_LEN && ip_test(adr) == 0 ){
                    rec = R_IP_GW;
					consol_state = WRITE_CFG;
				}else consol_state = OUT_ERROR;
				step++;
				break;
			}
       			if (strcmp(consol_buf, cmd_ipmask)== 0){
				if ( strlen(adr) < SETIPLAN_LEN && ip_test(adr) == 0 ){
                    rec = R_IP_MASK;
					consol_state = WRITE_CFG;
				}else consol_state = OUT_ERROR;
				step++;
				break;
			}

			// команда не найдена
			step++;
			consol_state = OUT_ERROR;
			break;

		case WRITE_CFG:								// сохранение параметра с структуре а затем и flash
			printf_dos("Write configure&setup device -> enc_config. . .\r\n");
			if (env_write_value(rec, adr) != 0 )
				printf_dos("ERROR save env_config.\n\r");
			step++;
			consol_state = OUT_PROMT;
			break;
		}
	}

}

//------------------------------------------------------------------------------
// test s - string for 111.222.333.444
//
// return 0  -> IP OK
// return !0 -> IP ERROR
//
//------------------------------------------------------------------------------
int ip_test(const char * s)
{

    size_t len = 0;    // strlen(s)
    size_t i = 0;      // index
    size_t m = 0;      // index for st
    char c;            // char from S
    int t = 0;         // kolichestvo tochek
    char st[4];        // sobiraem stroku dla convertachii str->int
    int n = 0;         // n = atoi(st)


    // string to NULL
    if (s == 0) return 1;
    len = strlen(s);

    // string LEN = 0
    if (len == 0) return 1;
    // string LEN > 192.168.130.100
    if (len > 15) return 1;

    i = 0;
    while( 1 ){
        c = *(s + i);
        // esli Ne chislo i ne Tochka -> error
        if ( !(c >= 0x30 && c <= 0x39) && c != '.' && c != 0) return 1;

        if (c >= 0x30 && c <= 0x39){
            if (m == 3) return 2;    // eto budet 4 chislo -> error

            st[m] = c;
            m++;
            st[m] = 0;
        }

        if (c == '.' || c == 0){

            if (m == 0) return 3;  // poluchili podrad vtoruu Tochku -> error

            n = atoi(st);
            if (n > 255) return 4;
            st[0] = 0;

            m = 0;

            if (c == 0) break; 
            else t++;
        }

        i++;
    }

    if (t != 3) return 5;

    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Выводим байт по битам на консоль
// 
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void print_byte_to_bit(uint8_t b)
{
        uint8_t i;

        for (i=0x80; i!=0; i=i>>1)
                if ( (b & i) > 0)   
                        printf_dos("1 ");
                else
                        printf_dos("0 ");
}
