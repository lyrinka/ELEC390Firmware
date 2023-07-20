#ifndef LWTDAQ_H__
#define LWTDAQ_H__

#include "libhandler.h"
#include "libscheduler.h"
#include "libtask.h"

typedef struct {
	// 16 bits, 1x
	unsigned short vis; 
	// 13 bits, 18x
	unsigned short uv; 
} LWTDAQ_Measurement_t; 

typedef struct {
	// Compressed (2^4-1) x 2^(2^4-1)
	unsigned char vis; 
	// Truncated (2^8-1)
	unsigned char uv; 
} LWTDAQ_CompressedMeasurement_t; 

typedef struct {
	Task_t task; 
	LWTDAQ_Measurement_t measurement; 
	unsigned int minutes; 
	LWTDAQ_CompressedMeasurement_t compressedMeasurement; 
	unsigned char seconds; 
	unsigned char started;
	unsigned char sampleReady; 
	unsigned char timerTriggered; 
} LWTDAQ_t; 

extern LWTDAQ_t LWTDAQ; 

extern void LWTDAQ_Init(void); 
extern void LWTDAQ_Start(void); 
extern void LWTDAQ_StartDelayed(unsigned int delayms); 

__weak void LWTDAQ_Callback(void); 

#endif
