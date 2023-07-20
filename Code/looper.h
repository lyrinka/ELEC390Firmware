#ifndef LOOPER_H__
#define LOOPER_H__

#include "libhandler.h"
#include "libscheduler.h"

typedef struct {
	Handler_t handler; 
	Scheduler_t scheduler; 
	unsigned int exit; 
} Looper_t; 

extern Looper_t MainLooper; 

extern void MainLooper_Init(void); 

extern void MainLooper_Entry(void); 

#define MainLooper_Submit(runnable) Handler_Submit(&MainLooper.handler, runnable)
#define MainLooper_SubmitDelayed(runnable, time) Scheduler_SubmitDelayed(&MainLooper.scheduler, runnable, time)
#define MainLooper_GetTickAmount() Scheduler_GetTickCount(&MainLooper.scheduler)

#endif
