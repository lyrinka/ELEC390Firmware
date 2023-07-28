#include "mainthread.h"

#include "libsys.h"
#include "libi2c.h"
#include "liblptim.h"

#include "blethread.h"

MainThread_State_t MainThread_State; 

/* -------- LWT instantiation -------- */
#define MAINTHREAD_STACK_SIZE 512
unsigned char MainThread_Stack[MAINTHREAD_STACK_SIZE]; 

void MainThread_Init(void) {
	// Data structure
	MainThread_State.seconds = 0; 
	
	// Event system
	void MainThread_EventInit(void); 
	MainThread_EventInit(); 
	
	// I2C
	I2C_HWInit(); 
	
	// LPTIM
	LPTIM_Init(); 
	
	// LWT
	void MainThread_Entry(void); 
	LWT_New(&MainThread_State.lwt, MainThread_Stack, MAINTHREAD_STACK_SIZE, MainThread_Entry); 
}

void MainThread_Start(void) {
	MainLooper_Submit2(LWT_Dispatch1, &MainThread_State.lwt, 0); 
}


/* -------- Event system -------- */
#define EV_TIMEBASE		1
#define EV_DELAY_DONE 2
#define EV_I2C_DONE		3

void MainThread_EventInit(void) {
	MainThread_State.flags = 0; 
}

void MainThread_WaitOnFlagBitmap(unsigned int bitmap) {
	while(!(MainThread_State.flags & bitmap)) {
		int ev = LWT_Yield1(); 
		if(ev == 0 || ev > 32) continue; 
		MainThread_State.flags |= 0x1 << (ev - 1); 
	}
	MainThread_State.flags &= ~bitmap; 
}

void MainThread_WaitOnFlag(int ev) {
	if(ev == 0 || ev > 32) return; 
	unsigned int bitmap =  0x1 << (ev - 1); 
	MainThread_WaitOnFlagBitmap(bitmap); 
}

void MainThread_CooperativeYield(void) {
	MainLooper_Submit2(LWT_Dispatch1, &MainThread_State.lwt, 0); 
	LWT_Yield1(); 
}


/* -------- Time delay -------- */
void MainThread_Delay(int tick) {
	MainLooper_SubmitDelayed2(LWT_Dispatch2, &MainThread_State.lwt, EV_DELAY_DONE, tick); 
	MainThread_WaitOnFlag(EV_DELAY_DONE); 
}


/* -------- Time base -------- */
void MainThread_WaitForTimeBase(void) {
	MainThread_WaitOnFlag(EV_TIMEBASE); 
}

// External weak function linkage -> liblptim.c
// IRQ context!!
void LPTIM_Callback(void) {
	MainLooper_Submit2(LWT_Dispatch2, &MainThread_State.lwt, EV_TIMEBASE); 
}

/* -------- I2C methods -------- */
void MainThread_I2CBlockingSession(void) {
	I2C_Session(); 
	MainThread_WaitOnFlag(EV_I2C_DONE); 
}

// External weak function linkage -> libi2c.c
// IRQ context!!
void I2C_SessionDoneCallback(int error) {
	MainLooper_Submit2(LWT_Dispatch2, &MainThread_State.lwt, EV_I2C_DONE); 
}

void MainThread_I2CWriteRegister1(unsigned char devaddr, unsigned char regaddr, unsigned char data) {
	unsigned char buf[1]; 
	I2C_Data.devaddr = devaddr; 
	I2C_Data.oplen = 1; 
	I2C_Data.opcode = regaddr; 
	I2C_Data.wrlen = 1; 
	I2C_Data.wrbuf = buf; 
	I2C_Data.rdlen = 0; 
	buf[0] = data; 
	MainThread_I2CBlockingSession(); 
}

unsigned int MainThread_I2CReadRegisters3(unsigned char devaddr, unsigned char regaddr) {
	unsigned char buf[3]; 
	I2C_Data.devaddr = devaddr; 
	I2C_Data.oplen = 1; 
	I2C_Data.opcode = regaddr; 
	I2C_Data.wrlen = 0; 
	I2C_Data.rdlen = 3; 
	I2C_Data.rdbuf = buf; 
	MainThread_I2CBlockingSession(); 
	unsigned int data = 0; 
	data |= buf[0]; 
	data |= buf[1] << 8; 
	data |= buf[2] << 16; 
	return data; 
}


/* -------- Sensor library: LTR390 -------- */
#define I2C_ADDR_LTR390 0xA6

unsigned int LTR390_Meas_VIS(void) { 
	// Conversion factor (raw 16 bits): 2.4, we do 4-4 compression
	MainThread_I2CWriteRegister1(I2C_ADDR_LTR390, 0x04, 0x46); // 16bit, 25ms
	MainThread_I2CWriteRegister1(I2C_ADDR_LTR390, 0x05, 0x00); // 1x
	MainThread_I2CWriteRegister1(I2C_ADDR_LTR390, 0x00, 0x02); // VIS measurement
	MainThread_Delay(40); 
	MainThread_I2CWriteRegister1(I2C_ADDR_LTR390, 0x00, 0x00); // Stop
	return MainThread_I2CReadRegisters3(I2C_ADDR_LTR390, 0x0D); 	
}

unsigned int LTR390_Meas_UV(void) { 
	// Conversion factor (raw 13 bits): 1/10.9375, we only take 8 bits
	MainThread_I2CWriteRegister1(I2C_ADDR_LTR390, 0x04, 0x56); // 13bit, 12.5ms
	MainThread_I2CWriteRegister1(I2C_ADDR_LTR390, 0x05, 0x04); // 18x
	MainThread_I2CWriteRegister1(I2C_ADDR_LTR390, 0x00, 0x0A); // UV measurement
	MainThread_Delay(30); 
	MainThread_I2CWriteRegister1(I2C_ADDR_LTR390, 0x00, 0x00); // Stop
	return MainThread_I2CReadRegisters3(I2C_ADDR_LTR390, 0x10); 	
}


/* -------- Data acquisition and algorithms -------- */
DAQ_OptiMeasCM_t DAQ_Compress(DAQ_OptiMeas_t meas) {
	DAQ_OptiMeasCM_t meas2; 
	// Truncate 8
	if(meas.uv > 0xFF) meas.uv = 0xFF; 
	meas2.uv = meas.uv; 
	// Compressed 4-4
	int bits = 32 - __clz(meas.vis); 
	if(bits < 4) bits = 4; 
	int exp = bits - 4; 
	int dig; 
	if(exp > 15) {
		meas2.vis = 0xFF; 
	}
	else {
		if(exp == 0) 
			dig = meas.vis; 
		else {
			dig = meas.vis >> (exp - 1); 
			if(dig & 1) dig += 1; 
			dig >>= 1; 
			if(dig & ~0xF) {
				dig >>= 1; 
				exp++; 
			}
		}
		meas2.vis = dig << 4 | exp; 
		if(meas2.vis == 0xFF) 
			meas2.vis = 0xEF; 
	}
	return meas2; 
}


/* -------- Main thread logic -------- */
void MainThread_PerformOpticalMeasurements(void) {
	MainThread_State.daq.optical.uv  = LTR390_Meas_UV(); 
	MainThread_State.daq.optical.vis = LTR390_Meas_VIS(); 
	MainThread_State.daq.opticalCM = DAQ_Compress(MainThread_State.daq.optical); 
}

void MainThread_PerformBatteryMeasurements(void) {
	// TODO
	return; 
}

void MainThread_PerformOpticalEstimations(void) {
	// TODO
	return; 
}

void MainThread_Entry(void) {
	BleThread_Start(); 
	MainThread_CooperativeYield(); 
	MainThread_State.daq.opticalHistory.index = 0; 
	MainThread_State.daq.opticalHistory.sample = 0; 
	for(;;) {
		// Wait for 1 second time base
		LED_Blue_Off(); 
		MainThread_WaitForTimeBase(); 
		LED_Blue_On(); 
		
		// Perform optical measurements every 1 second
		MainThread_PerformOpticalMeasurements(); 
		MainThread_SubmitRTOpticalMeas(MainThread_State.daq.opticalCM, MainThread_State.seconds); 
		
		// Perform battery measurements every 30 seconds
		if(!(MainThread_State.seconds % 30)) {
			MainThread_PerformBatteryMeasurements(); 
			MainThread_SubmitBatteryMeas(MainThread_State.daq.battery); 
		}
		
		// Perform estimations every 60 (configurable) seconds
		MainThread_State.daq.opticalHistory.history[MainThread_State.daq.opticalHistory.index] = MainThread_State.daq.optical; 
		if(++MainThread_State.daq.opticalHistory.index >= DAQ_OPTICAL_EVAL_INTERVAL) {
			MainThread_State.daq.opticalHistory.index = 0; 
			MainThread_PerformOpticalEstimations(); 
			MainThread_SubmitEstimatedOpticalMeas(MainThread_State.daq.opticalHistory.estimatedCM, MainThread_State.daq.opticalHistory.sample++); 
		}
		
		// Update timers
		++MainThread_State.seconds; 
	}
}

__weak void MainThread_SubmitRTOpticalMeas(DAQ_OptiMeasCM_t meas, unsigned int timestamp) {
	return; 
}

__weak void MainThread_SubmitBatteryMeas(DAQ_BattMeas_t meas) {
	return; 
}

__weak void MainThread_SubmitEstimatedOpticalMeas(DAQ_OptiMeasCM_t meas, unsigned int sample) {
	return; 
}
