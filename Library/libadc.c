#include <stm32g071xx.h>

#include "libadc.h"

unsigned short ADC_Data; 

void ADC_PowerOn(void) {
	ADC_Data = 0; 
	// ADC pin: PB11 (CH15)
	// Enable pin: PB12
	GPIOB->BRR = 0x1000; 
	GPIOB->OTYPER |= 0x1000; 
	GPIOB->MODER = GPIOB->MODER & 0xFC3FFFFF | 0x01C00000; 
	
	// Exit LPRUN
	PWR->CR1 &= ~0x4000; 
	
	RCC->APBENR2 |= RCC_APBENR2_ADCEN; 
	RCC->APBRSTR2 |=  RCC_APBRSTR2_ADCRST; 
	RCC->APBRSTR2 &= ~RCC_APBRSTR2_ADCRST; 
	__DSB(); 
	
	ADC1->CFGR1 = 0x00001000; 
	ADC1->CFGR2 = 0xE0000000; 
	ADC1->SMPR = 0x7; 
	ADC1->CHSELR = 0x00008000; 
	
	ADC1->ISR = 0xFFFFFFFF; 
	ADC1->IER = 0x0805; 
	
	NVIC_SetPriority(ADC1_COMP_IRQn, 2 << 6); 
	NVIC_EnableIRQ(ADC1_COMP_IRQn); 
	
	ADC1->CR = 0x10000000; 
}

void ADC_Convert(void) {
	GPIOB->BRR = 0x1000; 
	ADC1->CR |= ADC_CR_ADCAL; 
}

void ADC1_COMP_IRQHandler(void) {
	int flag = ADC1->ISR; 
	if(flag & ADC_ISR_EOCAL) {
		ADC1->ISR = ADC_ISR_EOCAL; 
		ADC1->CR |= ADC_CR_ADEN; 
	}
	if(flag & ADC_ISR_ADRDY) {
		ADC1->ISR = ADC_ISR_ADRDY; 
		ADC1->CR |= ADC_CR_ADSTART; 
	}
	if(flag & ADC_ISR_EOC) {
		ADC1->ISR = ADC_ISR_EOC; 
		ADC_Data = ADC1->DR; 
		ADC1->CR = ADC_CR_ADDIS; 
		// Disable switch
		GPIOB->BSRR = 0x1000; 
		// Enter LPRUN
		PWR->CR1 |= 0x4000; 
	}
}

unsigned int ADC_Cleanup(void) {
	RCC->APBRSTR2 |=  RCC_APBRSTR2_ADCRST; 
	RCC->APBRSTR2 &= ~RCC_APBRSTR2_ADCRST; 
	RCC->APBENR2 &= ~RCC_APBENR2_ADCEN; 
	return ((unsigned int)ADC_Data * VREF_FACTOR) >> 16; 
}
