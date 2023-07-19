#ifndef LWTDAQ_H__
#define LWTDAQ_H__

#include "libhandler.h"
#include "libscheduler.h"
#include "libtask.h"

typedef struct {
	Task_t task; 
	int busy; 
	struct {
		unsigned long long tick; 
		unsigned int uv; 
		unsigned int vis; 
	} meas; 
	void * callback; 
} LWTDAQ_State_t; 

extern LWTDAQ_State_t LWTDAQ_State; 

extern void LWTDAQ_Init(void * callback); 
extern void LWTDAQ_Trigger(void); 

#endif
