#include <stm32g071xx.h>

#include "looper.h"

#include "libsys.h"

void func1(void * f1, void * f2) {
	MainLooper_Submit(f1); 
	MainLooper_SubmitDelayed(f2, 50); 
}

void func2(void) {
	MainLooper_Submit2(func1, LED_Blue_On, LED_Blue_Off); 
	MainLooper_SubmitDelayed2(func1, LED_Green_On, LED_Green_Off, 500); 
	MainLooper_SubmitDelayed(func2, 1000); 
}

int main(void) {
	Sys_Init(); 
	
	MainLooper_Init(); 
	
	MainLooper_Submit(func2); 
	
	MainLooper_Run(); 
}

