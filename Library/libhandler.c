#include <stm32g071xx.h>

#include "libhandler.h"

#include "critregion.h"


void Handler_New(Handler_t * handler, Handler_RunnableWrapper_t * storage, unsigned int size) {
	handler->queue = storage; 
	handler->capacity = size; 
	handler->head = 0; 
	handler->tail = 0; 
	handler->size = 0; 
	handler->maxSizeReached = 0; 
	handler->totalSubmits = 0; 
	handler->totalFetches = 0; 
	handler->totalExecutes = 0; 
	handler->totalOverflows = 0; 
}

int Handler_Submit1(Handler_t * handler, void * runnable) {
	Handler_RunnableWrapper_t wrapper = {
		runnable, 0, 0, 
	}; 
	return Handler_Submit3(handler, wrapper); 
}

int Handler_Submit2(Handler_t * handler, void * runnable, void * context, int param) {
	Handler_RunnableWrapper_t wrapper = {
		runnable, context, param, 
	}; 
	return Handler_Submit3(handler, wrapper); 
}

int Handler_Submit3(Handler_t * handler, Handler_RunnableWrapper_t wrapper) {
	critEnter(); 
	if(handler->size >= handler->capacity) {
		// TODO: how do we log queue overflow?
		handler->totalOverflows++; 
		critExit(); 
		return HANDLER_SUBMIT_QUEUE_FULL; 
	}
	unsigned int head = handler->head; 
	handler->queue[head] = wrapper; 
	if(++head >= handler->capacity) head = 0; 
	handler->head = head; 
	if(++handler->size > handler->maxSizeReached) 
		handler->maxSizeReached = handler->size; 
	handler->totalSubmits++; 
	critExit(); 
	return HANDLER_SUBMIT_SUCCESS; 
}

int Handler_Fetch(Handler_t * handler, Handler_RunnableWrapper_t * wrapper) {
	critEnter(); 
	handler->totalFetches++; 
	if(handler->size == 0) {
		critExit(); 
		return HANDLER_FETCH_QUEUE_EMPTY; 
	}
	unsigned int tail = handler->tail; 
	*wrapper = handler->queue[tail]; 
	if(++tail >= handler->capacity) tail = 0; 
	handler->tail = tail; 
	handler->size--; 
	handler->totalExecutes++; 
	critExit(); 
	return HANDLER_FETCH_SUCCESS; 
}
