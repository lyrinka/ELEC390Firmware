#ifndef LIBPACKETS_H__
#define LIBPACKETS_H__

#include "libpacket.h"

#include "lwtdaq.h"

#define PacketInNewSample_ID 0x20
#define PacketInNewSample_Length 7
extern void PacketInNewSample(Packet_t * packet, unsigned char * payload, unsigned int min, unsigned char sec, LWTDAQ_CompressedMeasurement_t meas); 

#endif
