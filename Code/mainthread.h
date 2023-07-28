#ifndef MAINTHREAD_H__
#define MAINTHREAD_H__

#include "liblwt.h"
#include "looper.h"

typedef struct {
	LWT_t lwt; 
	volatile unsigned int flags; 
	unsigned int minutes; 
	unsigned char seconds; 
} MainThread_State_t; 

extern MainThread_State_t MainThread_State; 

extern void MainThread_Init(void); 

extern void MainThread_Start(void); 

#endif
