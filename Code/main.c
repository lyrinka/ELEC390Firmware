#include "mainthread.h"
#include "blethread.h"

#include "libsys.h"


int main(void) {
	Sys_Init(); 
	
	LWT_Init(); 
	MainLooper_Init(); 
	
	MainThread_Init(); 
	BleThread_Init(); 
	
	MainThread_Start(); 
	
	MainLooper_Run(); 
}
