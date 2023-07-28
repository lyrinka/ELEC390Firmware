#ifndef LIBUART_H__
#define LIBUART_H__

// Type: Profiling object
typedef struct {
	unsigned int rx; 
	unsigned int tx; 
	unsigned int overruns; 
} UART_Profiling_t; 

// Object: profiling object
extern UART_Profiling_t UART_Profiling; 

extern void UART_Init(void); 

extern void UART_TriggerTx(void); 

#define UART_TXHANDLER_SUCCESS 0
#define UART_TXHANDLER_NODATA -1
__weak int UART_TxEmptyHandler(unsigned char * data);  // IRQ context!!
__weak void UART_RxFullHandler(unsigned char data, int overrun); // IRQ context!!

#endif
