#ifndef LOOPER_H__
#define LOOPER_H__

#include "libhandler.h"
#include "libscheduler.h"

// Type: Looper object
typedef struct {
	Handler_t handler; 
	Scheduler_t scheduler; 
} Looper_t; 

// Object: This component has profiling data: MainLooper_Profiling
// Object: Looper object (singleton)
extern Looper_t MainLooper; 

// Procedure: MainLooper static initialization and object construction
extern void MainLooper_Init(void); 

// Procedure: MainLooper execution loop (blocks, never returns)
extern void MainLooper_Run(void); 

#define MainLooper_Submit MainLooper_Submit1
#define MainLooper_Submit1(runnable) 									Handler_Submit1(&MainLooper.handler, (runnable))
#define MainLooper_Submit2(runnable, context, param) 	Handler_Submit2(&MainLooper.handler, (runnable), (void *)(context), (int)(param))
#define MainLooper_Submit3(wrapper) 									Handler_Submit3(&MainLooper.handler, (wrapper))

#define MainLooper_SubmitDelayed MainLooper_SubmitDelayed1
#define MainLooper_SubmitDelayed1(runnable, delay) 									Scheduler_SubmitDelayed1(&MainLooper.scheduler, (runnable), (delay))
#define MainLooper_SubmitDelayed2(runnable, context, param, delay) 	Scheduler_SubmitDelayed2(&MainLooper.scheduler, (runnable), (void *)(context), (int)(param), (delay))
#define MainLooper_SubmitDelayed3(wrapper, delay) 									Scheduler_SubmitDelayed3(&MainLooper.scheduler, (wrapper), (delay))


#endif
