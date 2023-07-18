#include <stm32g071xx.h>

#include "libsys.h"
#include "libhandler.h"
#include "libtask.h"

#include "libi2c.h"

void delayms(int); 

unsigned char DAQTaskStack[512]; 
unsigned char RootHandlerBackedStorage[1024]; 


Task_t DAQTask; 
Handler_t RootHandler; 


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
	delayms(110); // TODO: how to handle this delay?
	I2C_WriteSingleRegister(I2C_ADDR_LTR390, 0x00, 0x00); // Stop
	return I2C_ReadRegisters3(I2C_ADDR_LTR390, 0x10); 	
}

unsigned int LTR390_Meas_VIS(void) {
	I2C_WriteSingleRegister(I2C_ADDR_LTR390, 0x04, 0x26); // 18bit, 2000ms
	I2C_WriteSingleRegister(I2C_ADDR_LTR390, 0x05, 0x01); // 3x
	I2C_WriteSingleRegister(I2C_ADDR_LTR390, 0x00, 0x02); // VIS measurement
	delayms(110); // TODO: how to handle this delay?
	I2C_WriteSingleRegister(I2C_ADDR_LTR390, 0x00, 0x00); // Stop
	return I2C_ReadRegisters3(I2C_ADDR_LTR390, 0x0D); 	
}



void callbackRunDAQ(Handler_t * handler, unsigned int param) {
	Task_Dispatch(&DAQTask); 
}

void I2C_StateChangeCallback(int error) {
	Handler_Post(&RootHandler, callbackRunDAQ, error); 
}

void DAQTaskFunc(Task_t * self) {
	for(;;) {
		unsigned int uvi = LTR390_Meas_UV(); 
		unsigned int vis = LTR390_Meas_VIS(); 
		__nop(); 
	}
}



int main(void) {
	Sys_Init(); 
	I2C_HWInit(); 
	LED_Blue_On(); 
	while(!Sys_LSEReady()); 
	LED_Blue_Off(); 
	
	Task_Init(); 
	Task_InitializeTask(&DAQTask, DAQTaskStack, sizeof(DAQTaskStack), DAQTaskFunc); 
	
	Handler_Init(&RootHandler, RootHandlerBackedStorage, sizeof(RootHandlerBackedStorage)); 
	
	Handler_Post(&RootHandler, callbackRunDAQ, 0); 
	
	while(1) {
		int status = Handler_Execute(&RootHandler); 
		if(status == HANDLER_EXECUTOR_EMPTY) {
			LED_Green_Off(); 
			for(int i = 0; i < 100; i++); 
		}
		else LED_Green_On(); 
	}
}



void delayms(int ms) {
	SysTick->CTRL = 0x4; 
	SysTick->LOAD = 999; 
	SysTick->VAL = 0x0; 
	SysTick->CTRL = 0x5; 
	for(int i = 0; i < ms; i++) 
		while(!(SysTick->CTRL & 0x10000)); 
	SysTick->CTRL = 0x4; 
}

