#ifndef MAINTHREAD_H__
#define MAINTHREAD_H__

#include "liblwt.h"
#include "looper.h"

#define DAQ_OPTICAL_EVAL_INTERVAL 60

typedef struct {
	// 16 bits, 1x
	unsigned short vis; 
	// 13 bits, 18x
	unsigned short uv; 
} DAQ_OptiMeas_t; 

typedef struct {
	// Compressed (2^4-1) x 2^(2^4-1)
	unsigned char vis; 
	// Truncated (2^8-1)
	unsigned char uv; 
} DAQ_OptiMeasCM_t; 

typedef struct {
	unsigned char voltage; 
	unsigned char percentage; 
} DAQ_BattMeas_t; 

typedef struct {
	LWT_t lwt; 
	volatile unsigned int flags; 
	unsigned int seconds; 
	struct {
		DAQ_OptiMeas_t optical; 
		DAQ_OptiMeasCM_t opticalCM; 
		DAQ_BattMeas_t battery; 
		struct {
			DAQ_OptiMeas_t history[DAQ_OPTICAL_EVAL_INTERVAL]; 
			DAQ_OptiMeasCM_t estimatedCM; 
			unsigned char counter; 
		} opticalHistory; 
	} daq; 
} MainThread_State_t; 

extern MainThread_State_t MainThread_State; 

extern void MainThread_Init(void); 

extern void MainThread_Start(void); 


__weak void MainThread_SubmitRTOpticalMeas(DAQ_OptiMeasCM_t meas); 
__weak void MainThread_SubmitBatteryMeas(DAQ_BattMeas_t meas); 
__weak void MainThread_SubmitEstimatedOpticalMeas(DAQ_OptiMeasCM_t meas); 

#endif
