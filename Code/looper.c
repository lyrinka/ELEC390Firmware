#include <stm32g071xx.h>

#include "looper.h"

#define MAIN_LOOPER_HANDLER_QUEUE_SIZE 128
Handler_Runnable_t MainLooperHandlerQueue[MAIN_LOOPER_HANDLER_QUEUE_SIZE]; 

#define MAIN_LOOPER_SCHEDULER_HEAP_SIZE 8
Scheduler_Promise_t MainLooperSchedulerHeap[MAIN_LOOPER_SCHEDULER_HEAP_SIZE]; 

Looper_t MainLooper; 

unsigned int profile_looper_handling_cnt = 0; 


void MainLooper_IdlePeriod(void); 

void SysTick_Init(void); 
void SysTick_Handler(void); 

void MainLooper_SchedulerExecution(Handler_t * handler, unsigned int tickAmount); 

void SysTick_Init(void) {
	// 10ms tick interval
	SysTick->CTRL = 0x4; 
	SysTick->LOAD = 10000 - 1; 
	SysTick->VAL = 0x0; 
	SysTick->CTRL = 0x7; 
}

void SysTick_Handler(void) {
	Handler_Post2(&MainLooper.handler, MainLooper_SchedulerExecution, 10); 
}

void MainLooper_SchedulerExecution(Handler_t * handler, unsigned int tickAmount) {
	Scheduler_AdvanceTick(&MainLooper.scheduler, tickAmount); 
}

void MainLooper_Entry(void * entryRunnable) {
	Handler_Init(&MainLooper.handler, MainLooperHandlerQueue, MAIN_LOOPER_HANDLER_QUEUE_SIZE); 
	Scheduler_Init(&MainLooper.scheduler, MainLooperSchedulerHeap, MAIN_LOOPER_SCHEDULER_HEAP_SIZE, &MainLooper.handler); 
	
	SysTick_Init(); 
	
	if(entryRunnable) 
		Handler_Post1(&MainLooper.handler, entryRunnable); 
	
	while(!MainLooper.exit) {
		int result = Handler_Execute(&MainLooper.handler); 
		if(result == HANDLER_EXECUTOR_PERFORMED) {
			profile_looper_handling_cnt++; 
			continue; 
		}
		MainLooper_IdlePeriod(); 
	}
}

void MainLooper_IdlePeriod(void) {
	__disable_irq(); 
	if(MainLooper.handler.size == 0) {
// TODO: debug under WFI
//	__wfi(); 
	for(int i = 0; i < 20; i++); 
	}
	__enable_irq(); 
}
