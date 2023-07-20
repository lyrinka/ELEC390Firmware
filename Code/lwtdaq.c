#include <stm32g071xx.h>

#include "lwtdaq.h"

#include "looper.h"

#include "libi2c.h"

struct {
	unsigned int acquisitions; 
	unsigned int spuriousI2CWaits; 
} LWTDAQ_Profiling; 

LWTDAQ_t LWTDAQ; 

#define LWTDAQ_STACK_SIZE 256
unsigned char LWTDAQ_Stack[LWTDAQ_STACK_SIZE]; 

void I2C_WriteSingleRegister(unsigned char devaddr, unsigned char regaddr, unsigned char data); 
unsigned int I2C_ReadRegisters3(unsigned char devaddr, unsigned char regaddr); 
unsigned int LTR390_Meas_UV(void); 
unsigned int LTR390_Meas_VIS(void); 

void LWTDAQ_Entry(void); 
void LWTDAQ_Init(void) {
	LWTDAQ.busy = 0; 
	Task_InitStack(&LWTDAQ.task, LWTDAQ_Stack, LWTDAQ_STACK_SIZE, LWTDAQ_Entry); 
}

void LWTDAQ_Resume(void) {
	Task_Dispatch(&LWTDAQ.task); 
}

void LWTDAQ_Trigger(void) {
	if(LWTDAQ.busy) return; 
	LWTDAQ.busy = 1; 
	MainLooper_Submit(LWTDAQ_Resume); 
}

void LWTDAQ_Delayms(int ms) {
	MainLooper_SubmitDelayed(LWTDAQ_Resume, ms); 
	Task_Yield(); 
}

void LWTDAQ_Entry(void) {
	for(;;) {
		LWTDAQ.busy = 1; 
		LWTDAQ.meas.uv   = LTR390_Meas_UV(); 
		LWTDAQ.meas.vis  = LTR390_Meas_VIS(); 
		LWTDAQ.meas.tick = MainLooper_GetTickAmount(); 
		LWTDAQ_Profiling.acquisitions++; 
		MainLooper_Submit(LWTDAQ_Callback); 
		LWTDAQ.busy = 0; 
		Task_Yield(); 
	}
}

__weak void LWTDAQ_Callback(void) {
	return; 
}

// I2C operations
void I2C_PollingEnsureIdle(void) {
	if(I2C_State.busy) {
		LWTDAQ_Profiling.spuriousI2CWaits++; 
		while(I2C_State.busy); 
	}
}

void I2C_WriteSingleRegister(unsigned char devaddr, unsigned char regaddr, unsigned char data) {
	unsigned char buf[1]; 
	I2C_PollingEnsureIdle(); 
	I2C_Data.devaddr = devaddr; 
	I2C_Data.oplen = 1; 
	I2C_Data.opcode = regaddr; 
	I2C_Data.wrlen = 1; 
	I2C_Data.wrbuf = buf; 
	I2C_Data.rdlen = 0; 
	buf[0] = data; 
	I2C_Session(); 
	Task_Yield(); 
}

unsigned int I2C_ReadRegisters3(unsigned char devaddr, unsigned char regaddr) {
	unsigned char buf[3]; 
	I2C_PollingEnsureIdle(); 
	I2C_Data.devaddr = devaddr; 
	I2C_Data.oplen = 1; 
	I2C_Data.opcode = regaddr; 
	I2C_Data.wrlen = 0; 
	I2C_Data.rdlen = 3; 
	I2C_Data.rdbuf = buf; 
	I2C_Session(); 
	Task_Yield(); 
	unsigned int data = 0; 
	data |= buf[0]; 
	data |= buf[1] << 8; 
	data |= buf[2] << 16; 
	return data; 
}

// Weakly linked to libi2c
void I2C_StateChangeCallback(int error) {
	MainLooper_Submit(LWTDAQ_Resume); 
}

// LTR390 operations
#define I2C_ADDR_LTR390 0xA6

unsigned int LTR390_Meas_UV(void) {
	I2C_WriteSingleRegister(I2C_ADDR_LTR390, 0x04, 0x26); // 18bit, 2000ms
	I2C_WriteSingleRegister(I2C_ADDR_LTR390, 0x05, 0x01); // 3x
	I2C_WriteSingleRegister(I2C_ADDR_LTR390, 0x00, 0x0A); // UV measurement
	LWTDAQ_Delayms(110); 
	I2C_WriteSingleRegister(I2C_ADDR_LTR390, 0x00, 0x00); // Stop
	return I2C_ReadRegisters3(I2C_ADDR_LTR390, 0x10); 	
}

unsigned int LTR390_Meas_VIS(void) {
	I2C_WriteSingleRegister(I2C_ADDR_LTR390, 0x04, 0x26); // 18bit, 2000ms
	I2C_WriteSingleRegister(I2C_ADDR_LTR390, 0x05, 0x01); // 3x
	I2C_WriteSingleRegister(I2C_ADDR_LTR390, 0x00, 0x02); // VIS measurement
	LWTDAQ_Delayms(110); 
	I2C_WriteSingleRegister(I2C_ADDR_LTR390, 0x00, 0x00); // Stop
	return I2C_ReadRegisters3(I2C_ADDR_LTR390, 0x0D); 	
}


