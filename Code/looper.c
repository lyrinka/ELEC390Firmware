#include <stm32g071xx.h>

#define DEBUG

#include "looper.h"

#define MAINLOOPER_HANDLER_QUEUE_SIZE 16

typedef struct {
	unsigned long long cycles; 
	unsigned long long sleeps; 
} MainLooper_Profiling_t; 

Looper_t MainLooper; 
MainLooper_Profiling_t MainLooper_Profiling; 

Handler_RunnableWrapper_t MainLooper_HandlerQueue[MAINLOOPER_HANDLER_QUEUE_SIZE]; 
//Scheduler_Promise_t MainLooper_SchedulerHeap[MAINLOOPER_SCHEDULER_HEAP_SIZE]; 

void MainLooper_Init(void) {
	Handler_Init(); 
	
	MainLooper_Profiling.cycles = 0; 
	MainLooper_Profiling.sleeps = 0; 
	
	MainLooper.tick = 0; 
	Handler_New(&MainLooper.handler, MainLooper_HandlerQueue, MAINLOOPER_HANDLER_QUEUE_SIZE); 
}

void MainLooper_Run(void) {
	void SysTick_Init(void); 
	void MainLooper_IdlePeriod(void); 
	
//SysTick_Init(); 
	for(;;) {
		MainLooper_Profiling.cycles++; 
		Handler_RunnableWrapper_t wrapper; 
		int status = Handler_Fetch(&MainLooper.handler, &wrapper); 
		if(status == HANDLER_FETCH_SUCCESS) {
			((void(*)(void *, int))wrapper.runnable)(wrapper.context, wrapper.param); 
		}
		else {
			MainLooper_IdlePeriod(); 
		}
	}
}

void MainLooper_IdlePeriod(void) {
	__disable_irq(); 
	if(MainLooper.handler.size == 0) {
#ifndef DEBUG
		__wfi(); 
#else
		while(!(SCB->ICSR & SCB_ICSR_VECTPENDING_Msk)); 
#endif
	}
	__enable_irq(); 
	__nop();
}

/*
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


*/
