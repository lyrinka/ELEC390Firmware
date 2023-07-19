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

extern void Scheduler_PostDelayed1(Scheduler_t * scheduler, unsigned int delay, void * func); 
extern void Scheduler_PostDelayed2(Scheduler_t * scheduler, unsigned int delay, void * func, unsigned int param); 
extern void Scheduler_PostDelayed3(Scheduler_t * scheduler, unsigned int delay, const Handler_Runnable_t * runnable); 

extern void Scheduler_AdvanceTick(Scheduler_t * scheduler, unsigned int amount); 

extern unsigned long long Scheduler_GetTickCount(const Scheduler_t * scheduler); 

#endif
