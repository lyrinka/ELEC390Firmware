#include <stm32g071xx.h>

#define DEBUG

#include "looper.h"

#define MAINLOOPER_HANDLER_QUEUE_SIZE 16
#define MAINLOOPER_SCHEDULER_HEAP_SIZE 8

typedef struct {
	unsigned long long cycles; 
	unsigned long long sleeps; 
} MainLooper_Profiling_t; 

Looper_t MainLooper; 
MainLooper_Profiling_t MainLooper_Profiling; 

Handler_RunnableWrapper_t MainLooper_HandlerQueue[MAINLOOPER_HANDLER_QUEUE_SIZE]; 
Scheduler_RunnableWrapper_t MainLooper_SchedulerHeap[MAINLOOPER_SCHEDULER_HEAP_SIZE]; 

void MainLooper_Init(void) {
	Handler_Init(); 
	Scheduler_Init(); 
	
	MainLooper_Profiling.cycles = 0; 
	MainLooper_Profiling.sleeps = 0; 
	
	Handler_New(&MainLooper.handler, MainLooper_HandlerQueue, MAINLOOPER_HANDLER_QUEUE_SIZE); 
	Scheduler_New(&MainLooper.scheduler, MainLooper_SchedulerHeap, MAINLOOPER_SCHEDULER_HEAP_SIZE, &MainLooper.handler); 
}

void MainLooper_Run(void) {
	void SysTick_Init(void); 
	void MainLooper_IdlePeriod(void); 
	
	SysTick_Init(); 
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

void SysTick_Init(void) {
	// IRQ Priority: 0
	SCB->SHP[1] = SCB->SHP[1] & 0x00FFFFFF; 
	// 10ms tick interval @ 1MHz
	SysTick->CTRL = 0x4; 
	SysTick->LOAD = 10000 - 1; 
	SysTick->VAL = 0x0; 
	SysTick->CTRL = 0x7; 
}

void SysTick_Handler(void) {
	// 10ms tick interval
	MainLooper_Submit2(Scheduler_AdvanceTicks, &MainLooper.scheduler, 10); 
}
