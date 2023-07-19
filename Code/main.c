#include <stm32g071xx.h>

#include "libsys.h"
#include "libi2c.h"

#include "looper.h"
#include "lwtdaq.h"

void callback1(void) {
//LED_Blue_On(); 
	LWTDAQ_Trigger(); 
	Scheduler_PostDelayed1(&MainLooper.scheduler, 1000, callback1); 
}

void callback2(void) {
	LED_Blue_Off(); 
}


int main(void) {
	Sys_Init(); 
	Task_Init(); 
	
	I2C_HWInit(); 
	
	LED_Blue_On(); 
	while(!Sys_LSEReady()); 
	LED_Blue_Off(); 
	
	LWTDAQ_Init(callback2); 
	
	MainLooper_Entry(callback1); 
	while(1); 
}


