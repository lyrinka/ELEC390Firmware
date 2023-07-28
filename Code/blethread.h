#ifndef BLETHREAD_H__
#define BLETHREAD_H__

#include "liblwt.h"
#include "looper.h"

#include "libstream.h"
#include "libpacket.h"

typedef struct {
	LWT_t lwt; 
	unsigned int flags; 
	struct {
		Stream_t tx; 
		Stream_t rx; 
	} streams; 
} BleThread_State_t; 

extern void BleThread_Init(void); 

extern void BleThread_Start(void); 

#endif
