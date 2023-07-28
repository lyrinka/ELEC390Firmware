#include "mainthread.h"
#include "blethread.h"

#include "libstorage.h"

#include "libsys.h"

// In MainLooper context
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

// In MainThread LWT context
void DAQ_SubmitBatteryMeas(DAQ_BattMeas_t meas) {
	if(!BleThread_IsConnected()) return; 
	// TODO: implement battery measurements
}

// In MainThread LWT context
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

// In MainThread LWT context
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

// In BleThread LWT context
void Connection_HandlePacketOutRequestSyncInfo(Packet_t * packet) {
	if(!BleThread_IsConnected()) return; 
	
	unsigned int sampleStart, sampleCount; 
	Storage_GetRecordedRange(&sampleStart, &sampleCount); 
	
	PacketInSyncInfo(
		packet, 
		sampleStart, 
		sampleCount, 
		DAQ_State.estSeconds, 
		DAQ_OPTICAL_EVAL_INTERVAL
	); 
	BleThread_TxPacket(packet); 
}
// In BleThread context
void Connection_HandlePacketOutRequestSyncData(Packet_t * packet) {
	if(!BleThread_IsConnected()) return; 
	
	unsigned int sampleStart = PacketOutRequestSyncData_ReadStartSample(packet); 
	int sampleCount = PacketOutRequestSyncData_ReadCount(packet); 
	
	PacketInSyncData(
		packet, 
		sampleStart, 
		sampleCount
	); 
	
	for(int i = 0; i < sampleCount; i++) {
		DAQ_OptiMeasCM_t meas; 
		int status = Storage_Read(sampleStart + i, &meas); 
		if(status != STORAGE_READ_SUCCESS) {
			meas.uv = 0xFF; 
			meas.vis = 0xFF; 
		}
		PacketInSyncData_WriteSample(
			packet, 
			i, 
			(unsigned char *)&meas
		); 
	}
	
	BleThread_TxPacket(packet); 
}

// In BleThread LWT context
void BleThread_HandlePacket(Packet_t * packet) {
	if(packet->dir != PACKET_DIR_OUT) return; 
	switch(packet->pid) {
		default: break; 
		case PacketOutRequestSyncInfo_ID: {
			if(packet->len != PacketOutRequestSyncInfo_Length) break; 
			Connection_HandlePacketOutRequestSyncInfo(packet); 
			break; 
		}
		case PacketOutRequestSyncData_ID: {
			if(packet->len != PacketOutRequestSyncData_Length) break; 
			Connection_HandlePacketOutRequestSyncData(packet); 
			break; 
		}
	}
}
