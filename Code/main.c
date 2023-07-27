#include <stm32g071xx.h>

#include "liblwt.h"

#include "libsys.h"

unsigned char stack1[256]; 

LWT_t thread1; 

void delayms(int); 
int func1(int state); 

int main(void) {
	Sys_Init(); 
	LWT_Init(); 
	LWT_Create(&thread1, stack1, sizeof(stack1), func1); 
	
	for(;;) {
		int param = LWT_Dispatch1(&thread1);
		if(param) delayms(param); 
	}
}

int func1(int state) {
//	for(;;) {
		LED_Blue_On(); 
		LWT_Yield2(10); 
		LED_Blue_Off(); 
//		LWT_Yield2(490); 
//	}
	return 490; 
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
