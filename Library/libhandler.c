#include <stm32g071xx.h>

#include "libhandler.h"

#define critEnter() unsigned int __X_IE = __get_PRIMASK(); __disable_irq()
#define critExit() __set_PRIMASK(__X_IE)

unsigned int profile_handling_cnt = 0; 

void Handler_Init(Handler_t * handler, Handler_Runnable_t * storage, unsigned int size) {
	handler->queue = storage; 
	handler->capacity = size; 
	handler->head = 0; 
	handler->tail = 0; 
	handler->size = 0; 
	handler->maxSizeReached = 0; 
}

void Handler_Post1(Handler_t * handler, void * func) {
	Handler_Post2(handler, func, 0); 
}

void Handler_Post2(Handler_t * handler, void * func, unsigned int param) {
	Handler_Runnable_t runnable; 
	runnable.func = (Handler_Func_t)func; 
	runnable.param = param; 
	Handler_Post3(handler, &runnable); 
}

void Handler_Post3(Handler_t * handler, const Handler_Runnable_t * runnable0) {
	critEnter(); 
	if(handler->size >= handler->capacity) {
		// TODO: how do we handle queue overflow?
		critExit(); 
		return; 
	}
	unsigned int head = handler->head; 
	Handler_Runnable_t * runnable = &handler->queue[head]; 
	if(++head >= handler->capacity) head = 0; 
	handler->head = head; 
	if(++handler->size > handler->maxSizeReached) 
		handler->maxSizeReached = handler->size; 
	runnable->func  = runnable0->func; 
	runnable->param = runnable0->param; 
	critExit(); 
}

__weak void Handler_Profiler_Enter(void * func); 
__weak void Handler_Profiler_Leave(void * func); 
int Handler_Execute(Handler_t * handler) {
	critEnter(); 
	if(handler->size == 0) {
		critExit(); 
		return HANDLER_EXECUTOR_EMPTY; 
	}
	unsigned int tail = handler->tail; 
	Handler_Runnable_t runnable = handler->queue[tail]; 
	if(++tail >= handler->capacity) tail = 0; 
	handler->tail = tail; 
	handler->size--;
	critExit(); 
	Handler_Profiler_Enter(runnable.func); 
	runnable.func(handler, runnable.param); 
	Handler_Profiler_Leave(runnable.func); 
	profile_handling_cnt++; 
	return HANDLER_EXECUTOR_PERFORMED; 
}

// profiling, using timer 7 and green LED
#include <stm32g071xx.h>
#include "libsys.h"

unsigned long long profile_handled_total_cycles = 0; 
unsigned int profile_handled_max_cycle = 0; 
void * profile_handled_max_cycle_at_func; 

__weak void Handler_Profiler_Enter(void * func) {
	LED_Blue_On(); 
	if(!(RCC->APBENR1 & RCC_APBENR1_TIM7EN)) {
		DBG->APBFZ1 |= DBG_APB_FZ1_DBG_TIM7_STOP; 
		RCC->APBENR1 |= RCC_APBENR1_TIM7EN; 
		__DSB(); 
		TIM7->CR1 = 0x08; 
		TIM7->PSC = 0; 
	}
	TIM7->CNT = 0; 
	TIM7->EGR = 0x1; 
	TIM7->CR1 |= 0x1; 
}

__weak void Handler_Profiler_Leave(void * func) {
	LED_Blue_Off(); 
	TIM7->CR1 &= ~0x1; 
	unsigned int value = TIM7->CNT; 
	if(!value) value = 0xFFFFFFFF; 
	profile_handled_total_cycles += value; 
	if(value > profile_handled_max_cycle) {
		profile_handled_max_cycle_at_func = func; 
		profile_handled_max_cycle = value; 
		__nop(); 
	}
}

