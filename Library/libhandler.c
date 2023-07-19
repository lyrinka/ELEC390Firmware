#include <stm32g071xx.h>

#include "libhandler.h"

// Profiling only, will be removed
#include "libsys.h"

#define critEnter() unsigned int __X_IE = __get_PRIMASK(); __disable_irq()
#define critExit() __set_PRIMASK(__X_IE)

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
	// LED toggling for profiling, will be removed
	LED_Green_On(); 
	runnable.func(handler, runnable.param); 
	LED_Green_Off(); 
	return HANDLER_EXECUTOR_PERFORMED; 
}
