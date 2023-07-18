#include <stm32g071xx.h>

#include "libsys.h"

void delayms(int ms) {
	SysTick->CTRL = 0x4; 
	SysTick->LOAD = 999; 
	SysTick->VAL = 0x0; 
	SysTick->CTRL = 0x5; 
	for(int i = 0; i < ms; i++) 
		while(!(SysTick->CTRL & 0x10000)); 
	SysTick->CTRL = 0x4; 
}

void test(void); 

int main(void) {
	Sys_Init(); 
	
	GPIOB->BRR = 1; 
	GPIOB->MODER = GPIOB->MODER & 0xFFFFFFFC | 0x00000001; 
	
	while(!Sys_LSEReady()); 
	GPIOB->BSRR = 1; 
	
	LPUART1_Init(); 
	
	while(1) {
		GPIOB->BRR = 1; 
		delayms(100); 
		GPIOB->BSRR = 1; 
		delayms(1900); 
	}
}

