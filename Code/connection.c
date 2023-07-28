#include "mainthread.h"
#include "blethread.h"

#include "libstorage.h"

#include "libsys.h"

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

void DAQ_SubmitBatteryMeas(DAQ_BattMeas_t meas) {
	if(!BleThread_IsConnected()) return; 
	// TODO: battery meas
}

void DAQ_SubmitRTOpticalMeas(DAQ_OptiMeasCM_t meas, unsigned int second) {
	if(!BleThread_IsConnected()) return; 
	Packet_t packet; 
	unsigned char payload[PacketInNewOpticalSample_Length]; 
	Packet_New(&packet, payload, PacketInNewOpticalSample_Length); 
	PacketInNewOpticalSample(
		&packet, 
		second, 
		(unsigned char *)&meas
	); 
	BleThread_TxPacket(&packet); 
}

void DAQ_SubmitEstimatedOpticalMeas(DAQ_OptiMeasCM_t meas, unsigned int sample) {
	
	if(!BleThread_IsConnected()) return; 
	Packet_t packet; 
	unsigned char payload[PacketInNewOpticalEstimation_Length]; 
	Packet_New(&packet, payload, PacketInNewOpticalEstimation_Length); 
	PacketInNewOpticalEstimation(
		&packet, 
		DAQ_OPTICAL_EVAL_INTERVAL, 
		sample, 
		(unsigned char *)&meas
	); 
	BleThread_TxPacket(&packet); 
}

void BleThread_HandlePacket(const Packet_t * packet) {
	return; 
}
