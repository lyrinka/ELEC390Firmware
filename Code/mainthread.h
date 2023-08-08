#ifndef MAINTHREAD_H__
#define MAINTHREAD_H__

#include "liblwt.h"
#include "looper.h"

#ifdef DEMO_MODE
#define DAQ_OPTICAL_EVAL_INTERVAL 10
#else
#define DAQ_OPTICAL_EVAL_INTERVAL 60
#endif

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

#define BATT_DISCHARGING 0
#define BATT_CHARGING 1
#define BATT_CHARGING_DONE 2
typedef struct {
	unsigned short unfilteredVoltage; 
	unsigned short voltage; 
	unsigned char percentage; 
	unsigned char state; 
} DAQ_BattMeas_t; 

typedef struct {
	LWT_t lwt; 
	volatile unsigned int flags; 
	unsigned int seconds; 
	unsigned int demoFakeUVDataWeight; 
} MainThread_State_t; 

typedef struct {
	DAQ_BattMeas_t battery; 
	DAQ_OptiMeas_t optical; 
	DAQ_OptiMeasCM_t opticalCM; 
	DAQ_OptiMeas_t estOptical; 
	DAQ_OptiMeasCM_t estOpticalCM; 
	unsigned char estSeconds; 
	unsigned int estSampleNumber; 
} DAQ_State_t; 

extern MainThread_State_t MainThread_State; 
extern DAQ_State_t DAQ_State; 

extern void MainThread_Init(void); 

extern void MainThread_Start(void); 

extern DAQ_OptiMeasCM_t MainThread_ProcessFakeUVData(DAQ_OptiMeasCM_t input); 

__weak void DAQ_SubmitRTOpticalMeas(DAQ_OptiMeasCM_t meas, unsigned int timestamp); 
__weak void DAQ_SubmitBatteryMeas(DAQ_BattMeas_t meas); 
__weak void DAQ_SubmitEstimatedOpticalMeas(DAQ_OptiMeasCM_t meas, unsigned int sample); 

#endif
