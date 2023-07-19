#ifndef LIBUARTBLE_H__
#define LIBUARTBLE_H__

#define UARTBLE_TXBUFFER_SIZE 1024
#define UARTBLE_RXBUFFER_SIZE 1024
#define UARTBLE_LINEBUFFER_SIZE 512

#include "libstream.h"

typedef struct {
	Stream_t txStream; 
	Stream_t rxStream; 
	struct {
		unsigned char * buffer; 
		unsigned int capacity; 
		unsigned char enabled; 
		unsigned char state; 
		unsigned int index; 
		unsigned int size; 
	} lineParser; 
} UARTBLE_t; 

extern UARTBLE_t UARTBLE; 

void UARTBLE_Init(void); 

#define UARTBLE_WRITE_SUCCESS 0
#define UARTBLE_WRITE_FAIL -1
extern int UARTBLE_Write(const unsigned char * line, unsigned int size); 

extern void UARTBLE_RxLineRelease(void); 

#endif
