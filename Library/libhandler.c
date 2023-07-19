#include <stm32g071xx.h>

#include "libhandler.h"

#define critEnter() unsigned int __X_IE = __get_PRIMASK(); __disable_irq()
#define critExit() __set_PRIMASK(__X_IE)

struct {
	unsigned long long submits; 
	unsigned long long executes; 
	unsigned int overflows; 
} Handler_Profiling; 

void Handler_Init(Handler_t * handler, Handler_Runnable_t * storage, unsigned int size) {
	handler->queue = storage; 
	handler->capacity = size; 
	handler->head = 0; 
	handler->tail = 0; 
	handler->size = 0; 
	handler->maxSizeReached = 0; 
}

void Handler_Submit(Handler_t * handler, Handler_Runnable_t runnable) {
	critEnter(); 
	if(handler->size >= handler->capacity) {
		// TODO: how do we handle queue overflow?
		Handler_Profiling.overflows++; 
		critExit(); 
		return; 
	}
	unsigned int head = handler->head; 
	handler->queue[head] = runnable; 
	if(++head >= handler->capacity) head = 0; 
	handler->head = head; 
	if(++handler->size > handler->maxSizeReached) 
		handler->maxSizeReached = handler->size; 
	Handler_Profiling.submits++; 
	critExit(); 
}

Handler_Runnable_t Handler_Fetch(Handler_t * handler) {
	critEnter(); 
	if(handler->size == 0) {
		critExit(); 
		return 0; 
	}
	unsigned int tail = handler->tail; 
	Handler_Runnable_t runnable = handler->queue[tail]; 
	if(++tail >= handler->capacity) tail = 0; 
	handler->tail = tail; 
	handler->size--;
	critExit(); 
	Handler_Profiling.executes++; 
	return runnable; 
}
