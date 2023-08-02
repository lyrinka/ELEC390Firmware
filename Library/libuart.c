#include <stm32g071xx.h>

#include "libuart.h"

UART_Profiling_t UART_Profiling; 

void UART_Init(void) {
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
	
	// Set USART2 at 57600 baud, enable Rx IRQ, disable Tx IRQ
	USART2->BRR = 17; 
	USART2->CR1 = 0x2000002D; 
	
	// Enable USART2 IRQ, priority 2
	NVIC_EnableIRQ(USART2_IRQn); 
	NVIC_SetPriority(USART2_IRQn, 2 << 6); 
}

void UART_TriggerTx(void) {
	USART2->CR1 |= USART_CR1_TXEIE_TXFNFIE; 
}

__weak int UART_TxEmptyHandler(unsigned char * data) { // IRQ context!!
	return UART_TXHANDLER_NODATA; 
}

__weak void UART_RxFullHandler(unsigned char data) { // IRQ context!!
	return; 
}

void USART2_IRQHandler(void) {
	unsigned int flags = USART2->ISR; 
	if((flags & USART_ISR_TXE_TXFNF) & (USART2->CR1 & USART_CR1_TXEIE_TXFNFIE)) {
		for(;;) {
			unsigned char data; 
			int status = UART_TxEmptyHandler(&data); 
			if(status == UART_TXHANDLER_SUCCESS) {
				USART2->TDR = data; 
				UART_Profiling.tx++; 
			}
			else {
				USART2->CR1 &= ~USART_CR1_TXEIE_TXFNFIE; 
				break; 
			}
			if(!(USART2->ISR & USART_ISR_TXE_TXFNF)) break; 
		}
	}
	if(flags & USART_ISR_RXNE_RXFNE) {
		for(;;) {
			UART_Profiling.rx++; 
			unsigned char data = USART2->RDR; 
			UART_RxFullHandler(data); 
			if(!(USART2->ISR & USART_ISR_RXNE_RXFNE)) break; 
		}
	}
	if(flags & USART_ISR_ORE) {
		// TODO: how do we deal with overruns?
		UART_Profiling.overruns++; 
		USART2->ICR = USART_ICR_ORECF; 
	}
}
