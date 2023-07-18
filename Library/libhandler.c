#include <stm32g071xx.h>

#include "libhandler.h"

typedef struct HandlerRunnable_s {
	void (*runnable)(Handler_t * handler, unsigned int param); 
	unsigned int param; 
} HandlerRunnable_t; 

#define critEnter() unsigned int __X_IE = __get_PRIMASK(); __disable_irq()
#define critExit() __set_PRIMASK(__X_IE)

void Handler_Init(Handler_t * handler, unsigned char * memory, unsigned int size) {
	// align to handler runnable size (8 bytes)
	if((unsigned int)memory & 0x7) {
		int offset = 0x8 - ((unsigned int)memory & 0x7); 
		memory += offset; 
		size -= offset; 
	}
	size >>= 3; 
	handler->queue = (HandlerRunnable_t *)memory; 
	handler->capacity = size; 
	handler->head = 0; 
	handler->tail = 0; 
	handler->size = 0; 
	handler->maxSizeReached = 0; 
}

void Handler_Post(Handler_t * handler, void (*runnable)(Handler_t * handler, unsigned int param), unsigned int param) {
	critEnter(); 
	if(handler->size >= handler->capacity) {
		// TODO: how do we handle queue overflow?
		critExit(); 
		return; 
	}
	unsigned int head = handler->head; 
	HandlerRunnable_t * element = &handler->queue[head]; 
	if(++head >= handler->capacity) head = 0; 
	handler->head = head; 
	if(handler->size++ > handler->maxSizeReached) 
		handler->maxSizeReached = handler->size; 
	element->runnable = runnable; 
	element->param = param; 
	critExit(); 
}

int Handler_Execute(Handler_t * handler) {
	critEnter(); 
	if(handler->size == 0) {
		critExit(); 
		return HANDLER_EXECUTOR_EMPTY; 
	}
	unsigned int tail = handler->tail; 
	HandlerRunnable_t element = handler->queue[tail]; 
	if(++tail >= handler->capacity) tail = 0; 
	handler->tail = tail; 
	handler->size--;
	critExit(); 
	element.runnable(handler, element.param); 
	return HANDLER_EXECUTOR_PERFORMED; 
}
