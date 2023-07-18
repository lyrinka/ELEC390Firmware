#include "looper.h"

unsigned char MainLooperHandlerBuffer[1024]; 

Looper_t MainLooper; 
unsigned int profile_looper_handling_cnt = 0; 

__weak void MainLooper_IdlePeriod(void); 
void MainLooper_Entry(void (*entryRunnable)(Handler_t * handler, unsigned int param)) {
	{
		unsigned char * memory = MainLooperHandlerBuffer; 
		unsigned int size = sizeof(MainLooperHandlerBuffer); 	
		Handler_Init(&MainLooper.handler, memory, size); 
	}
	if(entryRunnable) 
		Handler_Post(&MainLooper.handler, entryRunnable, 0); 
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
