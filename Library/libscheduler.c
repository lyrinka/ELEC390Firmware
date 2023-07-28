#include <stm32g071xx.h>

#include "libscheduler.h"

#include "critregion.h"

#define TARGET_TIME_INACTIVE 0xFFFFFFFFFFFFFFFF

void Scheduler_New(Scheduler_t * scheduler, Scheduler_RunnableWrapper_t * storage, unsigned int size, Handler_t * handler) {
	scheduler->handler = handler; 
	scheduler->currentTick = 0; 
	scheduler->heap = storage; 
	scheduler->capacity = size; 
	scheduler->size = 0; 
	scheduler->maxSizeReached = 0; 
	scheduler->totalOverflows = 0; 
	scheduler->totalSubmit0s = 0; 
	scheduler->totalSubmits = 0; 
	scheduler->totalDispatches = 0; 
	scheduler->totalFailedDispatches = 0; 
	for(int i = 0; i < size; i++) 
		storage[i].targetTick = TARGET_TIME_INACTIVE; 
}

unsigned long long Scheduler_GetTick(const Scheduler_t * scheduler) {
	unsigned long long tick; 
	critEnter(); 
	tick = scheduler->currentTick; 
	critExit(); 
	return tick; 
}

int Scheduler_SubmitDelayed1(Scheduler_t * scheduler, void * runnable, int delay) {
	Handler_RunnableWrapper_t wrapper = {
		runnable, 0, 0, 
	}; 
	return Scheduler_SubmitDelayed3(scheduler, wrapper, delay); 
}

int Scheduler_SubmitDelayed2(Scheduler_t * scheduler, void * runnable, void * context, int param, int delay) {
	Handler_RunnableWrapper_t wrapper = {
		runnable, context, param, 
	}; 
	return Scheduler_SubmitDelayed3(scheduler, wrapper, delay); 
}

int Scheduler_SubmitDelayed3(Scheduler_t * scheduler, Handler_RunnableWrapper_t runnable, unsigned int delay) {
	critEnter(); 
#ifndef ALWAYS_POST_DELAYED
	if(delay == 0) {
		Handler_Submit3(scheduler->handler, runnable); 
		scheduler->totalSubmit0s++; 
		scheduler->totalSubmits++; 
		critExit(); 
		return SCHEDULER_SUBMIT_SUCCESS; 
	}
#endif
	// TODO: implement this with minheap
	// Find vacant slot
	int i = 0; 
	unsigned int size = scheduler->capacity; 
	for(; i < size; i++) {
		if(scheduler->heap[i].targetTick == TARGET_TIME_INACTIVE)
			break; 
	}
	if(i >= size) {
		// TODO: how do we log overflow?
		scheduler->totalOverflows++; 
		critExit(); 
		return SCHEDULER_SUBMIT_HEAP_FULL; 
	}
	if(++scheduler->size > scheduler->maxSizeReached) 
		scheduler->maxSizeReached = scheduler->size; 
	scheduler->heap[i].runnable = runnable; 
	scheduler->heap[i].targetTick = scheduler->currentTick + delay; 
	scheduler->totalSubmits++; 
	critExit(); 
	return SCHEDULER_SUBMIT_SUCCESS; 
}

int Scheduler_AdvanceTicks(Scheduler_t * scheduler, unsigned int amount) {
	int count = 0; 
	critEnter(); 
	unsigned long long current = scheduler->currentTick + amount; 
	scheduler->currentTick = current; 
	unsigned int size = scheduler->capacity; 
	for(int i = 0; i < size; i++) {
		if(scheduler->heap[i].targetTick == TARGET_TIME_INACTIVE) continue; 
		if(scheduler->heap[i].targetTick > current) continue; 
		int status = Handler_Submit3(scheduler->handler, scheduler->heap[i].runnable); 
		if(status == HANDLER_SUBMIT_SUCCESS) {
			scheduler->heap[i].targetTick = TARGET_TIME_INACTIVE; 
			scheduler->size--; 
			scheduler->totalDispatches++; 
			count++; 
		}
		else {
			// TODO: how do we deal with handler full problems?
			scheduler->totalFailedDispatches++; 
		}
	}
	critExit(); 
	return count; 
}
