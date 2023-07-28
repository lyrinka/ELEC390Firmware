#include "stm32g071xx.h"

#include "liblptim.h"

void LPTIM_Init(void) {
	RCC->APBENR1 |= RCC_APBENR1_LPTIM1EN; 
	__DSB(); 
	RCC->CCIPR = RCC->CCIPR & 0xFFF3FFFF | 0x000C0000; // LSE
	__DSB(); 
	LPTIM1->CR = 0x1; 
	LPTIM1->CFGR = 0x0; 
	LPTIM1->ARR = 32768 - 1; 
	LPTIM1->IER = 0x2; 
	NVIC_SetPriority(TIM6_DAC_LPTIM1_IRQn, 1 << 6); 
	NVIC_EnableIRQ(TIM6_DAC_LPTIM1_IRQn); 
	LPTIM1->CR |= 0x4; 
}

void TIM6_DAC_LPTIM1_IRQHandler(void) {
	LPTIM1->ICR = 0x2; 
	LPTIM_Callback(); 
}

__weak void LPTIM_Callback(void) {
	return; 
}
