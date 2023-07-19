#include <stm32g071xx.h>

#include "libsys.h"
#include "libi2c.h"

#include "looper.h"
#include "lwtdaq.h"

void callback1(void) {
	LED_Green_On(); 
	LWTDAQ_Trigger(LED_Green_Off); 
	MainLooper_SubmitDelayed(callback1, 1000); 
}

int main(void) {
	Sys_Init(); 
	Task_Init(); 
	
	I2C_HWInit(); 
	
	LED_Blue_On(); 
	while(!Sys_LSEReady()); 
	LED_Blue_Off(); 
	
	LWTDAQ_Init(); 
	
	MainLooper_Init(); 
	MainLooper_Submit(callback1); 
	MainLooper_Entry();  
	while(1); 
}


