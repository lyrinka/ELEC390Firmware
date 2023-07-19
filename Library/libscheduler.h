#ifndef LIBSHCEULDER_H__
#define LIBSHCEULDER_H__

#include "libhandler.h"

typedef struct {
	Handler_Runnable_t runnable; 
	unsigned long long targetTime; 
} Scheduler_Promise_t; 

typedef struct {
	unsigned long long currentTick; 
	Handler_t * handler; 
	Scheduler_Promise_t * heap; 
	unsigned int capacity; 
} Scheduler_t; 

extern void Scheduler_Init(Scheduler_t * scheduler, Scheduler_Promise_t * storage, unsigned int size, Handler_t * handler); 
extern unsigned long long Scheduler_GetTickCount(const Scheduler_t * scheduler); 

extern void Scheduler_SubmitDelayed(Scheduler_t * scheduler, Handler_Runnable_t func, unsigned int delay); 

extern void Scheduler_AdvanceTicks(Scheduler_t * scheduler, unsigned int amount); 

#endif
