#include "mainthread.h"

#include "libstorage.h"

#include "libsys.h"
#include "libi2c.h"
#include "liblptim.h"
#include "libadc.h"

#include "blethread.h"

MainThread_State_t MainThread_State; 
DAQ_State_t DAQ_State; 

/* -------- LWT instantiation -------- */
#define MAINTHREAD_STACK_SIZE 512
unsigned char MainThread_Stack[MAINTHREAD_STACK_SIZE]; 

void MainThread_Init(int useAlternateTimebase) {
	// Data structure
	MainThread_State.seconds = 0; 
	
	// Event system
	void MainThread_EventInit(void); 
	MainThread_EventInit(); 
	
	// I2C
	I2C_HWInit(); 
	
	// Timebase (LPTIM)
	if(!useAlternateTimebase) LPTIM_Init(); 
	else {
		void MainThread_AlternateTimebase(void); 
		MainLooper_SubmitDelayed(MainThread_AlternateTimebase, 1000); 
		LED_Red_On(); 
		MainLooper_SubmitDelayed(LED_Red_Off, 5000); 
	}
	
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

void MainThread_AlternateTimebase(void) {
	LPTIM_Callback(); 
	MainLooper_SubmitDelayed(MainThread_AlternateTimebase, 1000); 
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
void DAQ_PerformOpticalMeasurements(void) {
	DAQ_State.optical.uv   = LTR390_Meas_UV(); 
	DAQ_State.optical.vis = LTR390_Meas_VIS(); 
	DAQ_State.opticalCM = DAQ_Compress(DAQ_State.optical); 
}

int DAQ_PerformBatteryMeasurements(void) {
	static unsigned char g_connected = 0; 
	// Charging status
	char update = 0; 
	unsigned char prevState = DAQ_State.battery.state; 
	if(Sys_IsCharging()) {
		DAQ_State.battery.state = BATT_CHARGING; 
	}
	else if(Sys_IsChargingDone()) {
		DAQ_State.battery.state = BATT_CHARGING_DONE; 
	}
	else {
		DAQ_State.battery.state = BATT_DISCHARGING; 
	}
	if(DAQ_State.battery.state != prevState) update = 1; 
	
	// Force update when connection state changes
	switch(g_connected) {
		default: 
			if(BleThread_IsConnected()) g_connected = 1; 
			break; 
		case 1:
			update = 1; 
			g_connected = 2; 
			break; 
		case 2: 
			if(!BleThread_IsConnected()) g_connected = 0;
			break; 
	}
	
	// Battery voltage
	ADC_PowerOn(); 
	MainThread_Delay(10); 
	ADC_Convert(); 
	MainThread_Delay(10); 
	int mv = ADC_Cleanup(); 
	DAQ_State.battery.unfilteredVoltage = mv; 
	if(DAQ_State.battery.voltage != 0) 
		mv = (((unsigned int)DAQ_State.battery.voltage * 768) + mv * 256) >> 10; 
	DAQ_State.battery.voltage = mv; 
	
	// Battery percentage
	// 4.20V corresponds to 100%
	// 3.30V corresponds to 0% (or 1%)
	// TODO: use a proper battery SOC algorithm
	int percentage = (mv - 3300) / 9; 
	if(percentage <= 0) percentage = 1; 
	if(percentage > 100) percentage = 100; 
	DAQ_State.battery.percentage = percentage; 
	
	return update; 
}

void DAQ_InitializeOpticalEstimations(void) {
	DAQ_State.estSeconds = 0; 
	DAQ_State.estSampleNumber = 0; 
	DAQ_State.estOptical = (DAQ_OptiMeas_t){0, 0}; 
}


void DAQ_PerformOpticalEstimations(void) {
	// TODO: withstand 5 seconds of strong artificial light
	if(DAQ_State.optical.uv > DAQ_State.estOptical.uv) 
		DAQ_State.estOptical.uv = DAQ_State.optical.uv; 
	if(DAQ_State.optical.vis > DAQ_State.estOptical.vis) 
		DAQ_State.estOptical.vis = DAQ_State.optical.vis; 
}

void DAQ_FinalizeOpticalEstimations(void) {
	DAQ_State.estOpticalCM = DAQ_Compress(DAQ_State.estOptical); 
	DAQ_State.estOptical = (DAQ_OptiMeas_t){0, 0}; 
}

void MainThread_HandleDeviceTurnOff(void) {
	static unsigned char g_consecutives = 0; 
	if(Button1_Read() && Button2_Read()) g_consecutives++; 
	else g_consecutives = 0; 
	if(g_consecutives < 5) return; 
	LED_Red_On(); 
	MainLooper_SubmitDelayed(Sys_Shutdown, 500); 
}

void MainThread_Entry(void) {
	Storage_Init(); 
	
	BleThread_Start(); 
	MainThread_CooperativeYield(); 
	
	DAQ_InitializeOpticalEstimations(); 
	
	for(;;) {
		// Wait for 1 second time base
		MainThread_WaitForTimeBase(); 
		
		// Perform optical measurements every 1 second
		DAQ_PerformOpticalMeasurements(); 
		DAQ_SubmitRTOpticalMeas(DAQ_State.opticalCM, MainThread_State.seconds); 
		
		// Perform estimations every 60 (configurable) seconds
		DAQ_PerformOpticalEstimations(); 
		if(++DAQ_State.estSeconds >= DAQ_OPTICAL_EVAL_INTERVAL) {
			DAQ_State.estSeconds = 0; 
			DAQ_FinalizeOpticalEstimations(); 
			
			DAQ_State.estSampleNumber = Storage_Append(&DAQ_State.estOpticalCM); 
			DAQ_SubmitEstimatedOpticalMeas(DAQ_State.estOpticalCM, DAQ_State.estSampleNumber); 
		}
		
		// Perform battery measurements every second, report every 30 seconds
		int forceUpdateBatt = DAQ_PerformBatteryMeasurements(); 
		if(!(MainThread_State.seconds % 30) || forceUpdateBatt) {
			DAQ_SubmitBatteryMeas(DAQ_State.battery); 
		}
		
		// Test LED handling
		if(!(MainThread_State.seconds % 10)) {
			if(BleThread_IsConnected()) {
				LED_Green_On(); 
				MainLooper_SubmitDelayed(LED_Green_Off, 20); 
			}
			else {
				LED_Blue_On(); 
				MainLooper_SubmitDelayed(LED_Blue_Off, 20); 
			}
		}
		
		// Device turn-off handling
		MainThread_HandleDeviceTurnOff(); 
		
		// Update timers
		++MainThread_State.seconds; 
	}
}

__weak void DAQ_SubmitRTOpticalMeas(DAQ_OptiMeasCM_t meas, unsigned int timestamp) {
	return; 
}

__weak void DAQ_SubmitBatteryMeas(DAQ_BattMeas_t meas) {
	return; 
}

__weak void DAQ_SubmitEstimatedOpticalMeas(DAQ_OptiMeasCM_t meas, unsigned int sample) {
	return; 
}
