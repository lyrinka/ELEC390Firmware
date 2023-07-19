#ifndef LIBHANDLER_H__
#define LIBHANDLER_H__

struct Handler_Runnable_s; 

typedef struct {
	struct Handler_Runnable_s * queue; 
	unsigned int capacity; 
	unsigned int head; 
	unsigned int tail; 
	unsigned int size; 
	unsigned int maxSizeReached; 
} Handler_t; 

typedef void (*Handler_Func_t)(Handler_t * handler, unsigned int param); 

typedef struct Handler_Runnable_s {
	Handler_Func_t func; 
	unsigned int param; 
} Handler_Runnable_t; 


extern void Handler_Init(Handler_t * handler, Handler_Runnable_t * storage, unsigned int size); 

extern void Handler_Post1(Handler_t * handler, void * func); 
extern void Handler_Post2(Handler_t * handler, void * func, unsigned int param); 
extern void Handler_Post3(Handler_t * handler, const Handler_Runnable_t * runnable); 

#define HANDLER_EXECUTOR_EMPTY 0
#define HANDLER_EXECUTOR_PERFORMED 1
extern int Handler_Execute(Handler_t * handler); 

#endif
