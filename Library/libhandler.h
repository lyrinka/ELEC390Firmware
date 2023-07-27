#ifndef LIBHANDLER_H__
#define LIBHANDLER_H__

// Type: Handler runnable wrapper
typedef struct {
	void * runnable; 
	void * context; 
	int param; 
} Handler_RunnableWrapper_t; 

// Type: Handler object
typedef struct {
	Handler_RunnableWrapper_t * queue; 
	unsigned int capacity; 
	unsigned int head; 
	unsigned int tail; 
	unsigned int size; 
} Handler_t; 

// Object: This component has profiling data: Handler_Profiling

// Procedure: Handler static initialization
extern void Handler_Init(void); 

// Procedure: Handler object construction
extern void Handler_New(Handler_t * handler, Handler_RunnableWrapper_t * storage, unsigned int size); 

// Procedure: Handler execution submission
// Callable from anywhere (thread/handling modes)
#define HANDLER_SUBMIT_SUCCESS 0
#define HANDLER_SUBMIT_QUEUE_FULL -1
extern int Handler_Submit1(Handler_t * handler, void * runnable); 
extern int Handler_Submit2(Handler_t * handler, void * runnable, void * context, int param); 
extern int Handler_Submit3(Handler_t * handler, Handler_RunnableWrapper_t wrapper); 

// Procedure: Obtain next object task to be executed
// Usually called from kernel
#define HANDLER_FETCH_SUCCESS 0
#define HANDLER_FETCH_QUEUE_EMPTY -1
extern int Handler_Fetch(Handler_t * handler, Handler_RunnableWrapper_t * wrapper); 

#endif
