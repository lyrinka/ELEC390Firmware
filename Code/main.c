#include <stm32g071xx.h>

#include "liblwt.h"
#include "looper.h"

#include "libsys.h"

int main(void) {
	Sys_Init(); 
	
	LWT_Init(); 
	MainLooper_Init(); 
	
	MainThread_Init(); 
	
	MainLooper_Run(); 
}

