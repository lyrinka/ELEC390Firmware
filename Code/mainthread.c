#include "liblwt.h"
#include "looper.h"

#include "libsys.h"
#include "libi2c.h"

/* -------- LWT instantiation -------- */
LWT_t MainThread_LWTObject; 

#define MAINTHREAD_STACK_SIZE 512
unsigned char MainThread_Stack[MAINTHREAD_STACK_SIZE]; 

void MainThread_Entry(void); 

void MainThread_Init(void) {
	LWT_New(&MainThread_LWTObject, MainThread_Stack, MAINTHREAD_STACK_SIZE, MainThread_Entry); 
	MainLooper_Submit2(LWT_Dispatch1, &MainThread_LWTObject, 0); 
}


/* -------- Event system -------- */
#define EV_DELAY_DONE 1
#define EV_I2C_DONE		2

volatile unsigned int MainThread_Flags; 

void MainThread_EventInit(void) {
	MainThread_Flags = 0; 
	// Note that I2C starts as IDLE
	MainThread_Flags |= 0x1 << (EV_I2C_DONE - 1); 
}

void MainThread_WaitOnFlagBitmap(unsigned int bitmap) {
	while(!(MainThread_Flags & bitmap)) {
		int ev = LWT_Yield1(); 
		if(ev == 0 || ev > 32) continue; 
		MainThread_Flags |= 0x1 << (ev - 1); 
	}
	MainThread_Flags &= ~bitmap; 
}

void MainThread_WaitOnFlag(int ev) {
	if(ev == 0 || ev > 32) return; 
	unsigned int bitmap =  0x1 << (ev - 1); 
	MainThread_WaitOnFlagBitmap(bitmap); 
}

void MainThread_Delay(int tick) {
	MainLooper_SubmitDelayed2(LWT_Dispatch2, &MainThread_LWTObject, EV_DELAY_DONE, tick); 
	MainThread_WaitOnFlag(EV_DELAY_DONE); 
}


/* -------- I2C methods -------- */
void MainThread_I2CBlockingSession(void) {
	I2C_Session(); 
	MainThread_WaitOnFlag(EV_I2C_DONE); 
}

// External weak function linkage -> libi2c.c
// IRQ context!!
void I2C_SessionDoneCallback(int error) {
	MainLooper_Submit2(LWT_Dispatch2, &MainThread_LWTObject, EV_I2C_DONE); 
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


/* -------- Testing code -------- */

void MainThread_Entry(void) {
	for(;;) {
		LED_Blue_On(); 
		MainThread_Delay(20); 
		LED_Blue_Off(); 
		MainThread_Delay(480); 
	}
}

