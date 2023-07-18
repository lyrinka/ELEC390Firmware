#ifndef LOOPER_H__
#define LOOPER_H__

#include "libhandler.h"

typedef struct {
	Handler_t handler; 
	unsigned int exit; 
} Looper_t; 

extern Looper_t MainLooper; 

extern void MainLooper_Entry( void (*entryRunnable)(Handler_t * handler, unsigned int param)); 

#endif
