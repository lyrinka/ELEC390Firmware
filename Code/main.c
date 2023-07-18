#include <stm32g071xx.h>

#include "libsys.h"
#include "libtask.h"

void LED_Init(void) {
	GPIOB->BSRR = 0x0001; 
	GPIOB->MODER = GPIOB->MODER & 0xFFFFFFFC | 0x00000001; 
	GPIOA->BSRR = 0x00C0; 
	GPIOA->MODER = GPIOA->MODER & 0xFFFF0FFF | 0x00005000; 
}

void LED_Red_On(void) {
	GPIOA->BRR = 0x0080; 
}

void LED_Red_Off(void) {
	GPIOA->BSRR = 0x0080; 
}

void LED_Green_On(void) {
	GPIOA->BRR = 0x0040; 
}

void LED_Green_Off(void) {
	GPIOA->BSRR = 0x0040; 
}

void LED_Blue_On(void) {
	GPIOB->BRR = 0x0001; 
}

void LED_Blue_Off(void) {
	GPIOB->BSRR = 0x0001; 
}

void delayms(int ms) {
	SysTick->CTRL = 0x4; 
	SysTick->LOAD = 999; 
	SysTick->VAL = 0x0; 
	SysTick->CTRL = 0x5; 
	for(int i = 0; i < ms; i++) 
		while(!(SysTick->CTRL & 0x10000)); 
	SysTick->CTRL = 0x4; 
}

unsigned char stack2[512]; 
Task_t task2; 
void task2_func(Task_t * self); 

unsigned char stack3[512]; 
Task_t task3; 
void task3_func(Task_t * self); 

int main(void) {
	Sys_Init(); 
	
	LED_Init(); 
	LED_Blue_On(); 
	while(!Sys_LSEReady()); 
	LED_Blue_Off(); 
	
	Task_Init(); 
	
	Task_InitializeTask(&task2, stack2, sizeof(stack2), task2_func); 
	Task_InitializeTask(&task3, stack3, sizeof(stack3), task3_func); 
	
	int count = 0; 
	while(1) {
		LED_Blue_On(); 
		delayms(100); 
		LED_Blue_Off(); 
		delayms(900); 
		if(++count & 0x1) 
			Task_Dispatch(&task2); 
		else
			Task_Dispatch(&task3); 
	}
}

void task2_func(Task_t * self) {
	while(1) {
		LED_Red_On(); 
		delayms(100); 
		LED_Red_Off(); 
		delayms(900); 
		Task_Yield(); 
	}
}

void task3_func(Task_t * self) {
	while(1) {
		LED_Green_On(); 
		delayms(100); 
		LED_Green_Off(); 
		delayms(900); 
		Task_Yield(); 
	}
}

