#include <stm32g071xx.h>

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

void BleThread_HandleConnectionFlow(int isConnected) {
	if(isConnected) {
		MainLooper_SubmitDelayed(LED_Green_On, 0); 
		MainLooper_SubmitDelayed(LED_Green_Off, 500); 
	}
	else {
		MainLooper_SubmitDelayed(LED_Red_On, 0); 
		MainLooper_SubmitDelayed(LED_Red_Off, 500); 
	}
}

void BleThread_HandlePacket(const Packet_t * packet) {
	return; 
}
