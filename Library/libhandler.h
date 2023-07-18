#ifndef LIBHANDLER_H__
#define LIBHANDLER_H__

struct HandlerRunnable_s; 

typedef struct {
	struct HandlerRunnable_s * queue; 
	unsigned int capacity; 
	unsigned int head; 
	unsigned int tail; 
	unsigned int size; 
	unsigned int maxSizeReached; 
} Handler_t; 

extern void Handler_Init(Handler_t * handler, unsigned char * memory, unsigned int size); 

extern void Handler_Post(Handler_t * handler, void (*runnable)(Handler_t * handler, unsigned int param), unsigned int param); 

#define HANDLER_EXECUTOR_EMPTY 0
#define HANDLER_EXECUTOR_PERFORMED 1
extern int Handler_Execute(Handler_t * handler); 

#endif
