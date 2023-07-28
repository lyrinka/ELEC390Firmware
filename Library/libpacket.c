#include "libpacket.h"


extern void Packet_New(Packet_t * packet, unsigned char * payload, int capacity) {
	if(capacity > 255) capacity = 255; 
	packet->capacity = capacity; 
	packet->payload = payload; 
}

int PacketInNewOpticalSample(
	Packet_t * packet, 
	unsigned int seconds, 
	const unsigned char * meas2
) {
	if(packet->capacity < PacketInNewOpticalSample_Length) return PACKET_CONSTRUCT_FAIL; 
	packet->dir = PACKET_DIR_IN; 
	packet->pid = PacketInNewOpticalSample_ID; 
	packet->len = PacketInNewOpticalSample_Length; 
	packet->payload[0] = seconds >> 24; 
	packet->payload[1] = seconds >> 16; 
	packet->payload[2] = seconds >> 8; 
	packet->payload[3] = seconds; 
	packet->payload[4] = meas2[0]; 
	packet->payload[5] = meas2[1]; 
	return PACKET_CONSTRUCT_SUCCESS; 
}

int PacketInNewOpticalEstimation(
	Packet_t * packet, 
	unsigned char interval, 
	unsigned int sample, 
	const unsigned char * meas2
) {
	if(packet->capacity < PacketInNewOpticalEstimation_Length) return PACKET_CONSTRUCT_FAIL; 
	packet->dir = PACKET_DIR_IN; 
	packet->pid = PacketInNewOpticalEstimation_ID; 
	packet->len = PacketInNewOpticalEstimation_Length; 
	packet->payload[0] = sample >> 24; 
	packet->payload[1] = sample >> 16; 
	packet->payload[2] = sample >> 8; 
	packet->payload[3] = sample; 
	packet->payload[4] = meas2[0]; 
	packet->payload[5] = meas2[1]; 
	packet->payload[6] = interval; 
	return PACKET_CONSTRUCT_SUCCESS; 
}

int PacketInSyncInfo(
	Packet_t * packet, 
	unsigned int sampleStart, 
	unsigned int sampleCount, 
	unsigned char secondCounter, 
	unsigned char interval
) {
	if(packet->capacity < PacketInSyncInfo_Length) return PACKET_CONSTRUCT_FAIL; 
	packet->dir = PACKET_DIR_IN; 
	packet->pid = PacketInSyncInfo_ID; 
	packet->len = PacketInSyncInfo_Length; 
	packet->payload[0] = sampleStart >> 24; 
	packet->payload[1] = sampleStart >> 16; 
	packet->payload[2] = sampleStart >> 8; 
	packet->payload[3] = sampleStart; 
	packet->payload[4] = sampleCount >> 24; 
	packet->payload[5] = sampleCount >> 16; 
	packet->payload[6] = sampleCount >> 8; 
	packet->payload[7] = sampleCount; 
	packet->payload[8] = secondCounter; 
	packet->payload[9] = interval; 
	return PACKET_CONSTRUCT_SUCCESS; 
}
