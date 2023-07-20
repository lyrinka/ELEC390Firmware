#include <stm32g071xx.h>

#include "looper.h"

#include "libsys.h"
#include "libi2c.h"
#include "libuartble.h"

#include "lwtdaq.h"

void callback1(void) {
	LED_Green_On(); 
	LWTDAQ_Trigger(LED_Green_Off); 
	MainLooper_SubmitDelayed(callback1, 1000); 
}

void UARTBLE_RxLineCallback2(void) {
	int status = UARTBLE_Write(UARTBLE.lineParser.buffer, UARTBLE.lineParser.size); 
	if(status != UARTBLE_WRITE_SUCCESS) {
		MainLooper_SubmitDelayed(UARTBLE_RxLineCallback, 20); 
		return; 
	}
	UARTBLE_RxLineRelease(); 
}

void UARTBLE_RxLineCallback(void) {
	MainLooper_SubmitDelayed(UARTBLE_RxLineCallback2, 10000); 
}

int main(void) {
	Sys_Init(); 
	LED_Blue_On(); 
	while(!Sys_LSEReady()); 
	LED_Blue_Off(); 
	
	Task_Init(); 
	MainLooper_Init(); 
	
	I2C_HWInit(); 
	UARTBLE_Init(); 
	
	LWTDAQ_Init(); 
	
	MainLooper_Submit(callback1); 
	MainLooper_Entry(); 
	while(1); 
}


