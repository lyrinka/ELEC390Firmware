#ifndef LIBUARTBLE_H__
#define LIBUARTBLE_H__

#include "libstream.h"

typedef struct {
	Stream_t txStream; 
	Stream_t rxStream; 
	struct {
		unsigned char * buffer; 
		unsigned int capacity; 
		unsigned int size; 
		unsigned int index; 
		unsigned char enabled; 
		unsigned char state; 
	} lineParser; 
} UARTBLE_t; 

extern UARTBLE_t UARTBLE; 

void UARTBLE_Init(void); 

#define UARTBLE_WRITE_SUCCESS 0
#define UARTBLE_WRITE_FAIL -1
extern int UARTBLE_Write(const unsigned char * line, unsigned int size); 
extern int UARTBLE_WriteLine(const unsigned char * line, unsigned int size); 
extern int UARTBLE_WriteLineWithPrefix(char prefix, const unsigned char * line, unsigned int size); 

__weak void UARTBLE_RxLineCallback(void); 
extern void UARTBLE_RxLineRelease(void); 

#endif
