#include "looper.h"

#define MAIN_LOOPER_HANDLER_QUEUE_SIZE 128
Handler_Runnable_t MainLooperHandlerQueue[MAIN_LOOPER_HANDLER_QUEUE_SIZE]; 

Looper_t MainLooper; 
unsigned int profile_looper_handling_cnt = 0; 

__weak void MainLooper_IdlePeriod(void); 
void MainLooper_Entry(void * entryRunnable) {
	Handler_Init(&MainLooper.handler, MainLooperHandlerQueue, MAIN_LOOPER_HANDLER_QUEUE_SIZE); 
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

__weak void MainLooper_IdlePeriod(void) {
	__disable_irq(); 
	if(MainLooper.handler.size == 0)
		__wfi(); 
	__enable_irq(); 
}
