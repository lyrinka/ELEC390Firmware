#include "libpacket.h"


extern void Packet_New(Packet_t * packet, unsigned char * payload, int capacity) {
	if(capacity > 255) capacity = 255; 
	packet->capacity = capacity; 
	packet->payload = payload; 
}

int PacketInNewSample(
	Packet_t * packet, 
	unsigned int min, 
	unsigned int sec, 
	unsigned char * meas2
) {
	if(packet->capacity < PacketInNewSample_Length) return PACKET_CONSTRUCT_FAIL; 
	packet->dir = PACKET_DIR_IN; 
	packet->pid = PacketInNewSample_ID; 
	packet->len = PacketInNewSample_Length; 
	packet->payload[0] = min >> 24; 
	packet->payload[1] = min >> 16; 
	packet->payload[2] = min >> 8; 
	packet->payload[3] = min; 
	packet->payload[4] = sec; 
	packet->payload[5] = meas2[0]; 
	packet->payload[6] = meas2[1]; 
	return PACKET_CONSTRUCT_SUCCESS; 
}
