#ifndef LIBHANDLER_H__
#define LIBHANDLER_H__

typedef void (*Handler_Runnable_t)(void); 

typedef struct {
	Handler_Runnable_t * queue; 
	unsigned int capacity; 
	unsigned int head; 
	unsigned int tail; 
	unsigned int size; 
	unsigned int maxSizeReached; 
} Handler_t; 

extern void Handler_Init(Handler_t * handler, Handler_Runnable_t * storage, unsigned int size); 

extern void Handler_Submit(Handler_t * handler, Handler_Runnable_t func); 

extern Handler_Runnable_t Handler_Fetch(Handler_t * handler); 

#endif
