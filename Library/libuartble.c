#include <stm32g071xx.h>

#include "libuartble.h"

#include "looper.h"

#include "libmutex.h"

#define LINEPARSER_READLINE 0
#define LINEPARSER_WAITLINE 1
#define LINEPARSER_OVERFLOW 2

UARTBLE_t UARTBLE; 

Stream_Profiling_t UARTBLE_TxStream_Profiling; 
Stream_Profiling_t UARTBLE_RxStream_Profiling; 

unsigned char UARTBLE_TxBuffer[UARTBLE_TXBUFFER_SIZE]; 
unsigned char UARTBLE_RxBuffer[UARTBLE_RXBUFFER_SIZE]; 
unsigned char UARTBLE_LineBuffer[UARTBLE_LINEBUFFER_SIZE]; 

void UARTBLE_Init(void) {
	// Data structure init
	Stream_Init(&UARTBLE.txStream, UARTBLE_TxBuffer, UARTBLE_TXBUFFER_SIZE); 
	Stream_Init(&UARTBLE.rxStream, UARTBLE_RxBuffer, UARTBLE_RXBUFFER_SIZE); 
	Stream_AttachProfileObj(&UARTBLE.txStream, &UARTBLE_TxStream_Profiling); 
	Stream_AttachProfileObj(&UARTBLE.rxStream, &UARTBLE_RxStream_Profiling); 
	
	UARTBLE.lineParser.buffer = UARTBLE_LineBuffer; 
	UARTBLE.lineParser.capacity = UARTBLE_LINEBUFFER_SIZE; 
	UARTBLE.lineParser.enabled = 1; // TODO: change this back to 0
	UARTBLE.lineParser.index = 0; 
	UARTBLE.lineParser.size = 0; 
	UARTBLE.lineParser.state = LINEPARSER_READLINE; 
	
	// Clock USART2 from 1MHz PCLK
	RCC->CCIPR = RCC->CCIPR & 0xFFFFFFF3; 
	
	// Enable UART Pins:
	// PA2: BLE Rx, MCU Tx, AF1 (USART2TX)
	// PA3: BLE Tx, MCU Rx, AF1 (USART2RX)
	// No cross wiring
	GPIOA->AFR[0] = GPIOA->AFR[0] & 0xFFFF00FF | 0x00001100; 
	GPIOA->MODER = GPIOA->MODER & 0xFFFFFF0F | 0x000000A0; 
	
	// Enable and reset USART2 peripheral
	RCC->APBENR1  |=  RCC_APBENR1_USART2EN; 
	__DSB(); 
	RCC->APBRSTR1 |=  RCC_APBRSTR1_USART2RST; 
	__DSB(); 
	RCC->APBRSTR1 &= ~RCC_APBRSTR1_USART2RST; 
	__DSB(); 
	
	// Set USART2 at 9600 baud, enable Rx IRQ
	USART2->BRR = 104; 
	USART2->CR1 = 0x2D; 
	
	// Enable USART2 IRQ
	NVIC_EnableIRQ(USART2_IRQn); 
}

int UARTBLE_Write(const unsigned char * line, unsigned int size) {
	critEnter(); 
	if(size + 2 > Stream_GetSizeRemaining(&UARTBLE.txStream)) {
		critExit(); 
		return UARTBLE_WRITE_FAIL; 
	}
	for(int i = 0; i < size; i++) {
		Stream_Write(&UARTBLE.txStream, line[i]); 
	}
	Stream_Write(&UARTBLE.txStream, 0x0D); 
	Stream_Write(&UARTBLE.txStream, 0x0A); 
	USART2->CR1 |= USART_CR1_TXEIE_TXFNFIE; 
	critExit(); 
	return UARTBLE_WRITE_SUCCESS; 
}

void UARTBLE_LineParser(void); 
void UARTBLE_RxLineRelease(void) {
	UARTBLE.lineParser.enabled = 1; 
	MainLooper_Submit(UARTBLE_LineParser); 
}

void UARTBLE_LineParser(void) {
	if(!UARTBLE.lineParser.enabled) return; 
	for(;;) {
		int data = Stream_Read(&UARTBLE.rxStream); 
		if(data == STREAM_READ_EMPTY) return; 
		switch(UARTBLE.lineParser.state) {
			default: {
				UARTBLE.lineParser.state = LINEPARSER_READLINE; 
				UARTBLE.lineParser.index = 0; 
			}
			case LINEPARSER_READLINE: {
				if(data > '~' || data < ' ') {
					int index = UARTBLE.lineParser.index; 
					UARTBLE.lineParser.state = LINEPARSER_WAITLINE; 
//				UARTBLE.lineParser.index = 0; 
					if(index == 0) break; 
					UARTBLE.lineParser.size = index - 1; 
					UARTBLE.lineParser.enabled = 0; 
					MainLooper_Submit(UARTBLE_RxLineCallback); 
					break; 
				}
				int index = UARTBLE.lineParser.index; 
				if(index >= UARTBLE.lineParser.capacity) {
					UARTBLE.lineParser.state = LINEPARSER_OVERFLOW; 
//				UARTBLE.lineParser.index = 0; 
					break; 
				}
				UARTBLE.lineParser.buffer[index] = data; 
				UARTBLE.lineParser.index = index + 1; 
				break; 
			}
			case LINEPARSER_WAITLINE: {
				if(data > '~' || data < ' ') break; 
				UARTBLE.lineParser.state = LINEPARSER_READLINE; 
				UARTBLE.lineParser.index = 1; 
				UARTBLE.lineParser.buffer[0] = data; 
				break; 
			}
			case LINEPARSER_OVERFLOW: {
				if(data > '~' || data < ' ') 
					UARTBLE.lineParser.state = LINEPARSER_WAITLINE; 
				break; 
			}
		}
	}
}

__weak void UARTBLE_RxLineCallback(void) {
	UARTBLE_RxLineRelease(); 
}

void USART2_IRQHandler(void) {
	unsigned int flags = USART2->ISR; 
	if(flags & USART_ISR_TXE_TXFNF) {
		int data = Stream_Read(&UARTBLE.txStream); 
		if(data == STREAM_READ_EMPTY) {
			USART2->CR1 &= ~USART_CR1_TXEIE_TXFNFIE; 
		}
		else {
			USART2->TDR = (unsigned char)data; 
		}		
	}
	if(flags & USART_ISR_RXNE_RXFNE) {
		int data = USART2->RDR; 
		int status = Stream_Write(&UARTBLE.rxStream, data); 
		if(status == STREAM_WRITE_FULL) {
			// TODO: how do we deal with buffer overflow? 
		}
		if(UARTBLE.lineParser.enabled) // Optional checking
			MainLooper_Submit(UARTBLE_LineParser); 
	}
}

