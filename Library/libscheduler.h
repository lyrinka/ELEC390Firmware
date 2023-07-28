#ifndef LIBSHCEULDER_H__
#define LIBSHCEULDER_H__

#include "libhandler.h"

// Type: Scheduler runnable wrapper
typedef struct {
	Handler_RunnableWrapper_t runnable; 
	unsigned long long targetTick; 
} Scheduler_RunnableWrapper_t; 

// Type: Scheduler object
typedef struct {
	Handler_t * handler; 
	unsigned long long currentTick; 
	Scheduler_RunnableWrapper_t * heap; 
	unsigned int capacity; 
	unsigned int size; 
} Scheduler_t; 

// Object: This component has profiling data: Scheduler_Profiling

// Procedure: Scheduler static initialization
extern void Scheduler_Init(void); 

// Procedure: Scheduler object construction
extern void Scheduler_New(Scheduler_t * scheduler, Scheduler_RunnableWrapper_t * storage, unsigned int size, Handler_t * handler); 

// Procedure: Scheduler tick count retrieval
// Callable from anywhere (thread/handling modes)
extern unsigned long long Scheduler_GetTick(const Scheduler_t * scheduler); 

// Procedure: Scheduler execution submission
// Callable from anywhere (thread/handling modes)
#define SCHEDULER_SUBMIT_SUCCESS 0
#define SCHEDULER_SUBMIT_HEAP_FULL -1
extern int Scheduler_SubmitDelayed1(Scheduler_t * scheduler, void * runnable, int delay); 
extern int Scheduler_SubmitDelayed2(Scheduler_t * scheduler, void * runnable, void * context, int param, int delay); 
extern int Scheduler_SubmitDelayed3(Scheduler_t * scheduler, Handler_RunnableWrapper_t wrapper, unsigned int delay); 

// Procedure: Scheduler tick advancement with scheduling
// Returns number of tasks successfully submitted
extern int Scheduler_AdvanceTicks(Scheduler_t * scheduler, unsigned int amount); 

#endif
