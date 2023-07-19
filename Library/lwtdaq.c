#include <stm32g071xx.h>

#include "lwtdaq.h"

#include "looper.h"
#include "libi2c.h"

void I2C_WriteSingleRegister(unsigned char devaddr, unsigned char regaddr, unsigned char data); 
unsigned int I2C_ReadRegisters3(unsigned char devaddr, unsigned char regaddr); 
unsigned int LTR390_Meas_UV(void); 
unsigned int LTR390_Meas_VIS(void); 

#define LWTDAQ_STACK_SIZE 512
unsigned char LWTDAQ_Stack[LWTDAQ_STACK_SIZE]; 

LWTDAQ_State_t LWTDAQ_State; 

void LWTDAQ_Entry(Task_t * self); 
void LWTDAQ_Init(void * callback) {
	LWTDAQ_State.busy = 0; 
	LWTDAQ_State.callback = callback; 
	Task_InitializeTask(&LWTDAQ_State.task, LWTDAQ_Stack, LWTDAQ_STACK_SIZE, LWTDAQ_Entry); 
}

void LWTDAQ_Resume(void) {
	Task_Dispatch(&LWTDAQ_State.task); 
}

void LWTDAQ_Trigger(void) {
	if(LWTDAQ_State.busy) return; 
	Handler_Post1(&MainLooper.handler, LWTDAQ_Resume); 
}

void LWTDAQ_Delayms(int ms) {
	Scheduler_PostDelayed1(&MainLooper.scheduler, ms, LWTDAQ_Resume); 
	Task_Yield(); 
}

void LWTDAQ_Entry(Task_t * self) {
	for(;;) {
		LWTDAQ_State.meas.uv   = LTR390_Meas_UV(); 
		LWTDAQ_State.meas.vis  = LTR390_Meas_VIS(); 
		LWTDAQ_State.meas.tick = Scheduler_GetTickCount(&MainLooper.scheduler); 
		if(LWTDAQ_State.callback) 
			Handler_Post1(&MainLooper.handler, LWTDAQ_State.callback); 
		Task_Yield(); 
	}
}


// I2C operations
#define I2C_ADDR_LTR390 0xA6

void I2C_WriteSingleRegister(unsigned char devaddr, unsigned char regaddr, unsigned char data) {
	unsigned char buf[1]; 
	while(I2C_State.busy); 
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
	while(I2C_State.busy); 
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

void I2C_StateChangeCallback(int error) {
	Handler_Post1(&MainLooper.handler, LWTDAQ_Resume); 
}

