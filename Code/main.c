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


void MainThread_SubmitRTOpticalMeas(DAQ_OptiMeasCM_t meas, unsigned int second) {
	Packet_t packet; 
	unsigned char payload[PacketInNewSample_Length]; 
	Packet_New(&packet, payload, PacketInNewSample_Length); 
	PacketInNewSample(
		&packet, 
		second, 
		(unsigned char *)&meas
	); 
	BleThread_TxPacket(&packet); 
}

void MainThread_SubmitBatteryMeas(DAQ_BattMeas_t meas) {
	
	
}

void MainThread_SubmitEstimatedOpticalMeas(DAQ_OptiMeasCM_t meas, unsigned int sample) {
	
	
}

