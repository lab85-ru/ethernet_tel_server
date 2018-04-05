#ifndef __QUEUE_H
#define __QUEUE_H

#include <stdint.h>

//#define queue_data_len  unsigned long       // ukazivaem razradnost danih bla rabori ocheredi !!! nado bit vnimatelno
#define QUEUE_DATA_LEN  uint32_t//unsigned long//unsigned short       // ukazivaem razradnost danih bla rabori ocheredi !!! nado bit vnimatelno

//#define DISABLE_INT __disable_interrupt();
//#define ENABLE_INT __enable_interrupt();

struct queue_buffer
{
	unsigned char *queue; 		// sam kolchevoy buffer
	QUEUE_DATA_LEN len;			// dlinna buffera kolchevogo
	QUEUE_DATA_LEN in;		// input index to quque
	QUEUE_DATA_LEN out;		// output index from quque

	unsigned char *rw_buf;		// bufer ot kuda berem danii
	QUEUE_DATA_LEN rw_len;		// dlinna dannih
};



extern QUEUE_DATA_LEN free_queue( struct queue_buffer *queue );
extern QUEUE_DATA_LEN datalen_queue( struct queue_buffer *queue );
extern int push_data_queue( struct queue_buffer *queue );      // function raboti s massivom
extern int pop_data_queue( struct queue_buffer *queue );       // function raboti s massivom

extern int push_data_queue_b( struct queue_buffer *queue, unsigned char d );       // function raboti s 1 byte
extern int pop_data_queue_b( struct queue_buffer *queue, unsigned char *d );       // function raboti s 1 byte
extern int read_data_queue_b( struct queue_buffer *queue, unsigned char *d );       // chitaem 1 byte bez izvlechenia
extern int reset_queue( struct queue_buffer *queue );       // reset queue in+out=0

#endif
