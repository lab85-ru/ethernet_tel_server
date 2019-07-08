#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "main.h"
#include "printf_hal.h"
#include "env_config.h"
#include "time_hal.h"
#include "init_hardware.h"

#include "FreeRTOS.h"
#include "task.h"

#include "network.h"
#include "uip_arp.h"
#include "inet.h"

#ifndef DEBUG
#define DEBUG 0
#endif

extern int table_mu[ ];
extern int table_di[ ];
extern const char ERROR_R_ENV[];


extern unsigned char udp_paket_buf[];
extern device_status_t device_status;
extern struct uip_eth_addr mac_addr;
extern char txt_buf[];

//--------------------------------------------------------------
#define BUF ((struct uip_eth_hdr *)&uip_buf[0])
//--------------------------------------------------------------

//-----------------------------------------------------------------------------
// расчет напряжения на резистивном делителе
// результат - 0 хорошо, !0 ошибка.
// buf - строка на выход
// adc_in - данные от АЦП
// k1 и k2 - значение одного разряда с АЦП k1/k2=3.3/4096=8/10000
// mu - множитель, di - делитель, входного делителя напряжения
//-----------------------------------------------------------------------------
int calc_volt_rdiv(char *buf, const int adc_in, const int k1, const int k2, const int mu, const int di)
{
    long int r1,r2,r3,r4;

    if (k2 == 0 || di == 0){
        sprintf(buf, "---");
        return 1;
    }

    r1 = (adc_in * mu * k1);
    r2 = (di * k2);

    r3 = r1 / r2;      // целая часть, до запятой
    if(DEBUG) printf("r3=%d\n", r3);

    r4 = r1 - r3 * r2;
    if(DEBUG) printf("r41=%d\n", r4);

    r4 = (r4 * 100) / r2;// дробная часть после запятой
    if(DEBUG) printf("r42=%d\n", r4);

    sprintf(buf, "%d,%02d", r3, r4);

    return 0;
}

//-----------------------------------------------------------------------------
// загрузка значений Множителя и Делителя из I2C памяти в таблицу
//-----------------------------------------------------------------------------
void load_i2c_mul_div(void)
{
    int i;
    
    if(DEBUG) printf_dos("Load flash -> Table K.\n");
    for (i=0; i<ADC_CHANNEL_QUANTITY; i++){
        if (env_read_value(R_MUL_0 + i * 2, txt_buf)){printf_dos(ERROR_R_ENV); txt_buf[0] = '\0';}
        table_mu[i] = atoi(txt_buf);
        if (env_read_value(R_DIV_0 + i * 2, txt_buf)){printf_dos(ERROR_R_ENV); txt_buf[0] = '\0';}
        table_di[i] = atoi(txt_buf);
        if(DEBUG) printf_dos("Mul%d = %d, Div%d = %d\n", i, table_mu[i], i, table_di[i]);
    }
}

//------------------------------------------------------------------------------
// TASK UIP
//------------------------------------------------------------------------------
void vTask_uIP(void *pvParameters) 
{
    unsigned int ip;
    int i;
    uint32_t time_val_start = 0;                // текущее системное сремя мс, для обновления данных    
    uint32_t timer_val_arp = 0;                 // таймер для обработки ARP

    device_status_update_adc();                 // обновляем состоние входов выходов ADC
    device_status_update_gpio();                // обновляем состоние входов выходов GPIO
        
    timer_val_arp = time_get_ms_counter();      // tekuhee systemnoe vrema (ms);
    time_val_start = time_get_ms_counter();     // tekuhee systemnoe vrema (ms);

    printf_dos("\nLoad K for ADC.\n");
    load_i2c_mul_div();
    
    printf_dos("ETH init.\n");
    tap_init( mac_addr.addr );

    uip_init();
	uip_arp_init();

	uip_setethaddr(mac_addr);

	uip_ipaddr_t ipaddr;
    if(env_read_value(R_IP_DEVICE, txt_buf)){
        printf_dos("ERROR: init_hardware R_IP_DEVICE\n");
        txt_buf[0] = '\0';
    }

    if ((ip = inet_addr( (char const*)txt_buf)) == INADDR_NONE){
        uip_ipaddr(ipaddr, 192, 168, 12, 100);
    }else{
        uip_ipaddr(ipaddr, ip & 0xff, (ip>>8) & 0xff, (ip>>16) & 0xff, (ip>>24) & 0xff);
    }
        uip_sethostaddr(ipaddr);

    if(env_read_value(R_IP_GW, txt_buf)){
        printf_dos("ERROR: init_hardware R_IP_GW\n");
        txt_buf[0] = '\0';
    }
    if ((ip = inet_addr( (char const*)txt_buf)) == INADDR_NONE){
        uip_ipaddr(ipaddr, 192, 168, 12, 1);
    }else{
        uip_ipaddr(ipaddr, ip & 0xff, (ip>>8) & 0xff, (ip>>16) & 0xff, (ip>>24) & 0xff);
    }
	uip_setdraddr(ipaddr);        

    if(env_read_value(R_IP_MASK, txt_buf)){
        printf_dos("ERROR: init_hardware R_IP_MASK\n");
        txt_buf[0] = '\0';
    }
    
    if ((ip = inet_addr( (char const*)txt_buf)) == INADDR_NONE){
        uip_ipaddr(ipaddr, 255, 255, 255, 0);
    }else{
        uip_ipaddr(ipaddr, ip & 0xff, (ip>>8) & 0xff, (ip>>16) & 0xff, (ip>>24) & 0xff);
    }
       	uip_setnetmask(ipaddr);

    httpd_init();

        
	for (;;) {
        vTaskDelay(1);           
                
		uip_len = tap_recv_packet((uint8_t *) uip_buf, UIP_BUFSIZE);

		if (uip_len > 0) {
			if (BUF->type == htons(UIP_ETHTYPE_IP)) {
				uip_arp_ipin();
				uip_input();
				if (uip_len > 0) {
					uip_arp_out();
					tap_send_packet((uint8_t *) uip_buf, uip_len);
				}
			} else if (BUF->type == htons(UIP_ETHTYPE_ARP)) {
				uip_arp_arpin();
				if (uip_len > 0) {
					tap_send_packet((uint8_t *) uip_buf, uip_len);
				}
			}
		}else if(calcul_time_out(time_get_ms_counter(), time_val_start, PERIODIC_TIMER_INTERVAL) == TIR_OUT) {
            time_val_start = time_get_ms_counter();
                        
            device_status_update_adc();      // обновляем состоние входов выходов АЦП
            device_status_update_gpio();     // обновляем состоние входов выходов GPIO

            for(i = 0; i < UIP_CONNS; i++) {
	            uip_periodic(i);
	            /* If the above function invocation resulted in data that
	               should be sent out on the network, the global variable
	               uip_len is set to a value > 0. */
	            if(uip_len > 0) {
	                uip_arp_out();
	        		tap_send_packet((uint8_t *) uip_buf, uip_len);
                }
            }

#if UIP_UDP
            for(i = 0; i < UIP_UDP_CONNS; i++) {
                uip_udp_periodic(i);
                /* If the above function invocation resulted in data that
                   should be sent out on the network, the global variable
                   uip_len is set to a value > 0. */
                if(uip_len > 0) {
                    uip_arp_out();
			        tap_send_packet((uint8_t *) uip_buf, uip_len);
                }
            }
#endif /* UIP_UDP */
      
            /* Call the ARP timer function every 10 seconds. */
            if(calcul_time_out(time_get_ms_counter(), timer_val_arp, ARP_TIMER_INTERVAL) == TIR_OUT) {
                timer_val_arp = time_get_ms_counter();
	        uip_arp_timer();

            }
         }
         taskYIELD();
	}// END FOR(;;)
}




