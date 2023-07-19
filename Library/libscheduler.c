#include <stm32g071xx.h>

#include "libscheduler.h"

#define TARGET_TIME_INACTIVE 0xFFFFFFFFFFFFFFFF

#define critEnter() unsigned int __X_IE = __get_PRIMASK(); __disable_irq()
#define critExit() __set_PRIMASK(__X_IE)

struct {
	unsigned long long submit0s; 
	unsigned long long submits; 
	unsigned long long executes; 
	unsigned int overflows; 
} Scheduler_Profiling; 

void Scheduler_Init(Scheduler_t * scheduler, Scheduler_Promise_t * storage, unsigned int size, Handler_t * handler) {
	scheduler->handler = handler; 
	scheduler->heap = storage; 
	scheduler->capacity = size; 
	scheduler->currentTick = 0; 
	for(int i = 0; i < size; i++) 
		storage[i].targetTime = TARGET_TIME_INACTIVE; 
}

unsigned long long Scheduler_GetTickCount(const Scheduler_t * scheduler) {
	unsigned long long tick; 
	critEnter(); 
	tick = scheduler->currentTick; 
	critExit(); 
	return tick; 
}

void Scheduler_SubmitDelayed(Scheduler_t * scheduler, Handler_Runnable_t runnable, unsigned int delay) {
#ifndef ALWAYS_POST_DELAYED
	if(delay == 0) {
		Scheduler_Profiling.submit0s++; 
		Handler_Submit(scheduler->handler, runnable); 
		return; 
	}
#endif
	critEnter(); 
	// Find vacant slot
	int i = 0; 
	unsigned int size = scheduler->capacity; 
	for(; i < size; i++) {
		if(scheduler->heap[i].targetTime == TARGET_TIME_INACTIVE)
			break; 
	}
	if(i >= size) {
		// TODO: how do we deal with overflow?
		Scheduler_Profiling.overflows++; 
		critExit(); 
		return; 
	}
	scheduler->heap[i].runnable = runnable; 
	scheduler->heap[i].targetTime = scheduler->currentTick + delay; 
	Scheduler_Profiling.submits++; 
	critExit(); 
}

void Scheduler_AdvanceTicks(Scheduler_t * scheduler, unsigned int amount) {
	critEnter(); 
	unsigned long long current = scheduler->currentTick + amount; 
	scheduler->currentTick = current; 
	unsigned int size = scheduler->capacity; 
	for(int i = 0; i < size; i++) {
		if(scheduler->heap[i].targetTime == TARGET_TIME_INACTIVE) continue; 
		if(scheduler->heap[i].targetTime > current) continue; 
		scheduler->heap[i].targetTime = TARGET_TIME_INACTIVE; 
		Handler_Submit(scheduler->handler, scheduler->heap[i].runnable); 
		Scheduler_Profiling.executes++; 
	}
	critExit(); 
}
