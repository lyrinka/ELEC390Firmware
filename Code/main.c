#include <stm32g071xx.h>

#include "looper.h"

#include "libsys.h"


void delayms(int); 


void func1(int * counter, int param) {
	if(*counter >= 4) return; 
	switch(param) {
		case 0: {
			LED_Blue_On(); 
			MainLooper_Submit2(func1, counter, 1); 
			break; 
		}
		case 1: {
			delayms(10); 
			MainLooper_Submit2(func1, counter, 2); 
			break; 
		}
		case 2: {
			LED_Blue_Off(); 
			MainLooper_Submit2(func1, counter, 3); 
			break; 
		}
		case 3: {
			delayms(490); 
			(*counter)++; 
			MainLooper_Submit2(func1, counter, 0); 
			break; 
		}
	}	
}

int main(void) {
	Sys_Init(); 
	
	MainLooper_Init(); 
	
	int variable = 0; 
	MainLooper_Submit2(func1, &variable, 0); 
	
	MainLooper_Run(); 
}


void delayms(int ms) {
	SysTick->CTRL = 0x4; 
	SysTick->LOAD = 1000 - 1; 
	SysTick->VAL = 0; 
	SysTick->CTRL = 0x5; 
	for(int i = 0; i < ms; i++) 
		while(!(SysTick->CTRL & 0x10000)); 
	SysTick->CTRL = 0x4; 
}
