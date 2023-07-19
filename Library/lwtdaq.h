#ifndef LWTDAQ_H__
#define LWTDAQ_H__

#include "libhandler.h"
#include "libscheduler.h"
#include "libtask.h"

#define LWTDAQ_STACK_SIZE 512

typedef struct {
	Task_t task; 
	int busy; 
	Handler_Runnable_t callback; 
	struct {
		unsigned long long tick; 
		unsigned int uv; 
		unsigned int vis; 
	} meas; 
} LWTDAQ_t; 

extern LWTDAQ_t LWTDAQ; 

extern void LWTDAQ_Init(void); 
extern void LWTDAQ_Trigger(Handler_Runnable_t callback); 

#endif
