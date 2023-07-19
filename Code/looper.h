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

extern void MainLooper_Entry(void * func); 

#endif
