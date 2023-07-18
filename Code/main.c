#include <stm32g071xx.h>

#include "libsys.h"
#include "libhandler.h"
#include "libtask.h"

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
unsigned char stack3[512]; 
unsigned char memory1[1024]; 


Task_t task2; 
void task2_func(Task_t * self); 

Task_t task3; 
void task3_func(Task_t * self); 

void callback1(Handler_t *, unsigned int); 
void callback2(Handler_t *, unsigned int); 
void callback3(Handler_t *, unsigned int); 

int main(void) {
	Sys_Init(); 
	
	LED_Blue_On(); 
	while(!Sys_LSEReady()); 
	LED_Blue_Off(); 
	
	Task_Init(); 
	
	Task_InitializeTask(&task2, stack2, sizeof(stack2), task2_func); 
	Task_InitializeTask(&task3, stack3, sizeof(stack3), task3_func); 
	
	Handler_t rootHandler; 
	
	Handler_Init(&rootHandler, memory1, sizeof(memory1)); 
	Handler_Post(&rootHandler, callback1, 0); 
	
	while(1) {
		int status = Handler_Execute(&rootHandler); 
		if(status == HANDLER_EXECUTOR_EMPTY) {
			LED_Green_On(); 
			for(int i = 0; i < 100; i++); 
		}
		else LED_Green_Off(); 
	}
}

void callback1(Handler_t * handler, unsigned int param) {
	LED_Blue_On(); 
	delayms(100); 
	LED_Blue_Off(); 
	delayms(400); 
	Handler_Post(handler, callback2, param); 
	Handler_Post(handler, callback2, param); 
}

void callback2(Handler_t * handler, unsigned int param) {
	LED_Red_On(); 
	delayms(100); 
	LED_Red_Off(); 
	delayms(400); 
	Handler_Post(handler, callback3, param); 
}


void callback3(Handler_t * handler, unsigned int param) {
	if(++param > 5) return; 
	Handler_Post(handler, callback1, param); 
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

