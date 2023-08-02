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
	
	LWT_Init(); 
	MainLooper_Init(); 
	
	MainThread_Init(); 
	BleThread_Init(); 
	
	MainThread_Start(); 
	
	MainLooper_Run(); 
}
