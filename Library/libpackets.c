#include "libpackets.h"

void PacketInNewSample(Packet_t * packet, unsigned char * payload, unsigned int min, unsigned char sec, LWTDAQ_CompressedMeasurement_t meas) {
	packet->dir = 1; 
	packet->pid = PacketInNewSample_ID; 
	packet->len = PacketInNewSample_Length; 
	packet->payload = payload; 
	payload[0] = min >> 24; 
	payload[1] = min >> 16; 
	payload[2] = min >> 8; 
	payload[3] = min; 
	payload[4] = sec; 
	payload[5] = meas.vis; 
	payload[6] = meas.uv; 
}
