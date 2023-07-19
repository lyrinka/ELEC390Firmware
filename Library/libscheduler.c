#include <stm32g071xx.h>

#include "libscheduler.h"

#define TARGET_TIME_INACTIVE 0xFFFFFFFFFFFFFFFF

#define critEnter() unsigned int __X_IE = __get_PRIMASK(); __disable_irq()
#define critExit() __set_PRIMASK(__X_IE)

void Scheduler_Init(Scheduler_t * scheduler, Scheduler_Promise_t * storage, unsigned int size, Handler_t * handler) {
	scheduler->handler = handler; 
	scheduler->heap = storage; 
	scheduler->capacity = size; 
	scheduler->currentTick = 0; 
	for(int i = 0; i < size; i++) 
		storage[i].targetTime = TARGET_TIME_INACTIVE; 
}

void Scheduler_PostDelayed1(Scheduler_t * scheduler, unsigned int delay, void * func) {
	Scheduler_PostDelayed2(scheduler, delay, func, 0); 
}

void Scheduler_PostDelayed2(Scheduler_t * scheduler, unsigned int delay, void * func, unsigned int param) {
	Handler_Runnable_t runnable; 
	runnable.func = (Handler_Func_t)func; 
	runnable.param = param; 
	Scheduler_PostDelayed3(scheduler, delay, &runnable); 
}

void Scheduler_PostDelayed3(Scheduler_t * scheduler, unsigned int delay, const Handler_Runnable_t * runnable) {
	if(delay <= 0) {
		Handler_Post3(scheduler->handler, runnable); 
		return; 
	}
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
		critExit(); 
		return; 
	}
	scheduler->heap[i].runnable = *runnable; 
	scheduler->heap[i].targetTime = scheduler->currentTick + delay; 
	critExit(); 
}

void Scheduler_AdvanceTick(Scheduler_t * scheduler, unsigned int amount) {
	critEnter(); 
	unsigned long long current = scheduler->currentTick + amount; 
	scheduler->currentTick = current; 
	unsigned int size = scheduler->capacity; 
	for(int i = 0; i < size; i++) {
		if(scheduler->heap[i].targetTime == TARGET_TIME_INACTIVE) continue; 
		if(scheduler->heap[i].targetTime > current) continue; 
		scheduler->heap[i].targetTime = TARGET_TIME_INACTIVE; 
		Handler_Post3(scheduler->handler, &scheduler->heap[i].runnable); 
	}
	critExit(); 
}

