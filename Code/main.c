#include "mainthread.h"
#include "blethread.h"

#include "libsys.h"

#include "libstorage.h"

int main(void) {
	// Fill array at high clock speed
	for(int i = 0; i < STORAGE_ARRAY_SIZE; i++) {
		Storage.array[i] = (DAQ_OptiMeasCM_t){
			0xFF, 0xFF, 
		}; 
	}
	
	// Application code
	Sys_Init(); 
	
	// LSE timeout handling
	int lseTimeoutCounter = 0; 
	int useAlternateTimebase = 0; 
	LED_Green_On(); 
	while(!Sys_LSEReady()) {
		lseTimeoutCounter++; 
		if(lseTimeoutCounter > 100000) {
			useAlternateTimebase = 1; 
			break; 
		}
	}
	LED_Green_Off(); 
	
	// System initialization
	LWT_Init(); 
	MainLooper_Init(); 
	
	MainThread_Init(useAlternateTimebase); 
	BleThread_Init(); 
	
	MainThread_Start(); 
	
	// System boot
	MainLooper_Run(); 
}
