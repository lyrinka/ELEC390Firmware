#include <stm32g071xx.h>

#include "lwtdaq.h"

#include "looper.h"

#include "libsys.h"
#include "libi2c.h"

#include "libstorage.h"

struct {
	unsigned int spuriousI2CWaits; 
} LWTDAQ_Profiling; 

LWTDAQ_t LWTDAQ; 

#define LWTDAQ_STACK_SIZE 512
unsigned char LWTDAQ_Stack[LWTDAQ_STACK_SIZE]; 

// Data handling functions
LWTDAQ_CompressedMeasurement_t LWTDAQ_CompressMeasurement(LWTDAQ_Measurement_t meas); 

// I2C functions
void I2C_WriteRegister1(unsigned char devaddr, unsigned char regaddr, unsigned char data); 
unsigned int I2C_ReadRegisters3(unsigned char devaddr, unsigned char regaddr); 

// LTR functions
unsigned int LTR390_Meas_UV(void); 
unsigned int LTR390_Meas_VIS(void); 

// LWTDAQ implementation
void LWTDAQ_Entry(void); 
void LWTDAQ_Init(void) {
	LWTDAQ.started = 0; 
	LWTDAQ.sampleReady = 0; 
	LWTDAQ.timerTriggered = 0; 
	
	LWTDAQ.minutes = 0; 
	LWTDAQ.seconds = 0; 
	
	LWTDAQ.ledMode = LWTDAQ_LEDMODE_OFFLINE; 
	
	Task_InitStack(&LWTDAQ.task, LWTDAQ_Stack, LWTDAQ_STACK_SIZE, LWTDAQ_Entry); 
}

void LWTDAQ_Resume(void) {
	Task_Dispatch(&LWTDAQ.task); 
}

void LWTDAQ_Start(void) {
	if(LWTDAQ.started) return; 
	LWTDAQ.started = 1; 
	MainLooper_Submit(LWTDAQ_Resume); 
}

void LWTDAQ_StartDelayed(unsigned int delayms) {
	if(LWTDAQ.started) return; 
	LWTDAQ.started = 1; 
	MainLooper_SubmitDelayed(LWTDAQ_Resume, delayms); 
}

void LWTDAQ_Delayms(int ms) {
	MainLooper_SubmitDelayed(LWTDAQ_Resume, ms); 
	Task_Yield(); 
}

void LWTDAQ_Entry(void) {
	// LPTIM init
	RCC->APBENR1 |= RCC_APBENR1_LPTIM1EN; 
	__DSB(); 
	RCC->CCIPR = RCC->CCIPR & 0xFFF3FFFF | 0x000C0000; // LSE
	__DSB(); 
	LPTIM1->CR = 0x1; 
	LPTIM1->CFGR = 0x0; 
	LPTIM1->ARR = 32768 - 1; 
	LPTIM1->IER = 0x2; 
	NVIC_EnableIRQ(TIM6_DAC_LPTIM1_IRQn); 
	LPTIM1->CR |= 0x4; 
	// Some other system functions
	Storage_Init(); 
	int ledCounter = 0; 
	for(unsigned char firstCycle = 1;; firstCycle = 0) {
		// Wait for LPTIM trigger
		while(!LWTDAQ.timerTriggered) Task_Yield(); 
		LWTDAQ.timerTriggered = 0; 
		// Real DAQ
		unsigned short vis = LTR390_Meas_VIS() & 0xFFFF; 
		unsigned short uv = LTR390_Meas_UV() & 0x1FFF; 
		LWTDAQ.measurement.vis = vis; 
		LWTDAQ.measurement.uv = uv; 
		LWTDAQ.compressedMeasurement 
			= LWTDAQ_CompressMeasurement(
				LWTDAQ.measurement
		); 
		char minuteOverflow = 0; 
		if(!firstCycle) {
			if(++LWTDAQ.seconds >= 60) {
				LWTDAQ.minutes++; 
				LWTDAQ.seconds = 0; 
				minuteOverflow = 1; 
			}
		}
		// TODO: data processing
		LWTDAQ.sampleReady = 1; 
		MainLooper_Submit(LWTDAQ_Callback); 
		if(minuteOverflow) {
			Storage_Append(LWTDAQ.compressedMeasurement); 
			__nop(); 
		}
		// LED handling
		if(!(LWTDAQ.ledMode & LWTDAQ_LEDMODE_OFF)) {
			switch(LWTDAQ.ledMode) {
				default: 
				case LWTDAQ_LEDMODE_OFFLINE: {
					if(!ledCounter) {
						LED_Blue_On(); 
						MainLooper_SubmitDelayed(LED_Blue_Off, 10); 
					}
					if(++ledCounter > 32) ledCounter = 0; 
					break; 
				}
				case LWTDAQ_LEDMODE_CONNECTED: {
					LED_Green_On(); 
					MainLooper_SubmitDelayed(LED_Green_Off, 1000); 
					LWTDAQ.ledMode = LWTDAQ_LEDMODE_ONLINE; 
					ledCounter = 1; 
					break; 
				}
				case LWTDAQ_LEDMODE_ONLINE: {
					if(!ledCounter) {
						LED_Green_On(); 
						MainLooper_SubmitDelayed(LED_Green_Off, 10); 
					}
					if(++ledCounter > 8) ledCounter = 0; 
					break; 
				}
				case LWTDAQ_LEDMODE_DISCONNECTED: {
					LED_Red_On(); 
					MainLooper_SubmitDelayed(LED_Red_Off, 1000); 
					LWTDAQ.ledMode = LWTDAQ_LEDMODE_OFFLINE; 
					ledCounter = 1; 
					break; 
				}
			}
		}
		// TODO: charging indication
		
	}
}

void TIM6_DAC_LPTIM1_IRQHandler(void) {
	LPTIM1->ICR = 0x2; 
	LWTDAQ.timerTriggered = 1; 
	MainLooper_Submit(LWTDAQ_Resume); 
}

__weak void LWTDAQ_Callback(void) {
	return; 
}

// Data handling
LWTDAQ_CompressedMeasurement_t LWTDAQ_CompressMeasurement(LWTDAQ_Measurement_t meas) {
	LWTDAQ_CompressedMeasurement_t meas2; 
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
			if(dig & 1) dig += 2; 
			dig >>= 1; 
		}
		meas2.vis = dig << 4 | exp; 
		if(meas2.vis == 0xFF) 
			meas2.vis = 0xEF; 
	}
	return meas2; 
}

// I2C operations
void I2C_PollingEnsureIdle(void) {
	if(I2C_State.busy) {
		LWTDAQ_Profiling.spuriousI2CWaits++; 
		while(I2C_State.busy); 
	}
}

void I2C_WriteRegister1(unsigned char devaddr, unsigned char regaddr, unsigned char data) {
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

// Conversion factor (raw 16 bits): 2.4, we do 4-4 compression
unsigned int LTR390_Meas_VIS(void) { 
	I2C_WriteRegister1(I2C_ADDR_LTR390, 0x04, 0x46); // 16bit, 25ms
	I2C_WriteRegister1(I2C_ADDR_LTR390, 0x05, 0x00); // 1x
	I2C_WriteRegister1(I2C_ADDR_LTR390, 0x00, 0x02); // VIS measurement
	LWTDAQ_Delayms(40); 
	I2C_WriteRegister1(I2C_ADDR_LTR390, 0x00, 0x00); // Stop
	return I2C_ReadRegisters3(I2C_ADDR_LTR390, 0x0D); 	
}

// Conversion factor (raw 13 bits): 1/10.9375, we only take 8 bits
unsigned int LTR390_Meas_UV(void) { 
	I2C_WriteRegister1(I2C_ADDR_LTR390, 0x04, 0x56); // 13bit, 12.5ms
	I2C_WriteRegister1(I2C_ADDR_LTR390, 0x05, 0x04); // 18x
	I2C_WriteRegister1(I2C_ADDR_LTR390, 0x00, 0x0A); // UV measurement
	LWTDAQ_Delayms(30); 
	I2C_WriteRegister1(I2C_ADDR_LTR390, 0x00, 0x00); // Stop
	return I2C_ReadRegisters3(I2C_ADDR_LTR390, 0x10); 	
}

