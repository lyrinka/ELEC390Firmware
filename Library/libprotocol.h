#ifndef LIBPROTOCOL_H__
#define LIBPROTOCOL_H__

#include "libfifo.h"
#include "libpacket.h"

typedef struct {
	Fifo_t txFifo; 
	Fifo_t rxFifo; 
	struct {
		unsigned char * buffer; 
		unsigned short capacity; 
		unsigned short index; 
	} linebuf; 
	int state; 
} Protocol_t; 

#define TXBUFFER_CAPACITY 1024
#define RXBUFFER_CAPACITY 2048
#define RXLINEBUF_CAPACITY 512

extern void Protocol_Init(void); 
extern int Protocol_GetTx(void); 
extern int Protocol_TxString(const char * str); 
extern int Protocol_TxPacket(const Packet_t * packet); 

#define RXTYPE_EMPTY 0
#define RXTYPE_PACKET 1
#define RXTYPE_STRING 2

#endif
