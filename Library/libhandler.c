#include <stm32g071xx.h>

#include "libhandler.h"

#include "critregion.h"

typedef struct {
	unsigned long long submits; 
	unsigned long long fetches; 
	unsigned long long executes; 
	unsigned int overflows; 
	unsigned int maxSizeReached; 
} Handler_Profiling_t; 

Handler_Profiling_t Handler_Profiling; 

void Handler_Init(void) {
	Handler_Profiling.submits = 0; 
	Handler_Profiling.fetches = 0; 
	Handler_Profiling.executes = 0; 
	Handler_Profiling.overflows = 0; 
	Handler_Profiling.maxSizeReached = 0; 
}

void Handler_New(Handler_t * handler, Handler_RunnableWrapper_t * storage, unsigned int size) {
	handler->queue = storage; 
	handler->capacity = size; 
	handler->head = 0; 
	handler->tail = 0; 
	handler->size = 0; 
}

int Handler_Submit1(Handler_t * handler, void * runnable) {
	return Handler_Submit2(handler, runnable, 0, 0); 
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
		Handler_Profiling.overflows++; 
		critExit(); 
		return HANDLER_SUBMIT_QUEUE_FULL; 
	}
	unsigned int head = handler->head; 
	handler->queue[head] = wrapper; 
	if(++head >= handler->capacity) head = 0; 
	handler->head = head; 
	if(++handler->size > Handler_Profiling.maxSizeReached) 
		Handler_Profiling.maxSizeReached = handler->size; 
	Handler_Profiling.submits++; 
	critExit(); 
	return HANDLER_SUBMIT_SUCCESS; 
}

int Handler_Fetch(Handler_t * handler, Handler_RunnableWrapper_t * wrapper) {
	critEnter(); 
	Handler_Profiling.fetches++; 
	if(handler->size == 0) {
		critExit(); 
		return HANDLER_FETCH_QUEUE_EMPTY; 
	}
	unsigned int tail = handler->tail; 
	*wrapper = handler->queue[tail]; 
	if(++tail >= handler->capacity) tail = 0; 
	handler->tail = tail; 
	handler->size--; 
	Handler_Profiling.executes++; 
	critExit(); 
	return HANDLER_FETCH_SUCCESS; 
}
