#include <stm32g071xx.h>

#include "looper.h"

#define MAINLOOPER_HANDLER_QUEUE_SIZE 16
#define MAINLOOPER_SCHEDULER_HEAP_SIZE 8

struct {
	unsigned long long cycles; 
} MainLooper_Profiling; 

Looper_t MainLooper; 

Handler_Runnable_t MainLooper_HandlerQueue[MAINLOOPER_HANDLER_QUEUE_SIZE]; 
Scheduler_Promise_t MainLooper_SchedulerHeap[MAINLOOPER_SCHEDULER_HEAP_SIZE]; 

void SysTick_Init(void); 
void SysTick_Handler(void); 
void MainLooper_SchedulerExecution(void); 
void MainLooper_IdlePeriod(void); 

void MainLooper_Init(void) {
	Handler_Init(&MainLooper.handler, MainLooper_HandlerQueue, MAINLOOPER_HANDLER_QUEUE_SIZE); 
	Scheduler_Init(&MainLooper.scheduler, MainLooper_SchedulerHeap, MAINLOOPER_SCHEDULER_HEAP_SIZE, &MainLooper.handler); 
}

void MainLooper_Entry(void) {
	SysTick_Init(); 
	while(!MainLooper.exit) {
		MainLooper_Profiling.cycles++; 
		Handler_Runnable_t runnable = Handler_Fetch(&MainLooper.handler); 
		if(runnable) runnable(); 
		else MainLooper_IdlePeriod(); 
	}
}

void SysTick_Init(void) {
	// 10ms tick interval
	SysTick->CTRL = 0x4; 
	SysTick->LOAD = 10000 - 1; 
	SysTick->VAL = 0x0; 
	SysTick->CTRL = 0x7; 
}

void SysTick_Handler(void) {
	MainLooper_Submit(MainLooper_SchedulerExecution); 
}

void MainLooper_SchedulerExecution(void) {
	Scheduler_AdvanceTicks(&MainLooper.scheduler, 10); 
}

void MainLooper_IdlePeriod(void) {
	__disable_irq(); 
	if(MainLooper.handler.size == 0) {
		__wfi(); 
	//while(!(SCB->ICSR & SCB_ICSR_VECTPENDING_Msk)); 
	}
	__enable_irq(); 
	__nop();
}
