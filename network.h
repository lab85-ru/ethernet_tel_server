/*
 * Simple common network interface that all network drivers should implement.
 */

#ifndef __NETWORK_H__
#define __NETWORK_H__


/*Initialize the network*/
//void tap_init(uint8_t *macadr);
#if ETH_KSZ8851==1
#include "ksz8851.h"

#define tap_init(x)  ksz8851Init(x)
/*Read from the network, returns number of read bytes*/
uint16_t tap_recv_packet(uint8_t *buf, uint16_t buflen);

/*Send using the network*/
void tap_send_packet(uint8_t *buf, uint16_t len);
#endif

#if ETH_ENC28J60==1
#include "enc28j60.h"

#define tap_init(x)          enc28j60_init(x)
#define tap_recv_packet(x,y) enc28j60_recv_packet(x, y)
#define tap_send_packet(x,y) enc28j60_send_packet(x, y)
#endif


/*Initialize the network with a mac addr*/
//void tap_init_mac(uint8_t* macaddr);


#endif /* __NETWORK_H__ */
