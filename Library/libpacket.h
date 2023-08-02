#ifndef LIBPACKET_H__
#define LIBPACKET_H__

#define PACKET_DIR_IN  1
#define PACKET_DIR_OUT 0

// Type: Packet object
typedef struct {
	// Direction of the packet with respect to the remote device. 
	// Zero implies OUT packet (host to MCU), non-zero implies IN packet (MCU to host). 
	unsigned char dir; 
	
	// Packet identifier, valid 0 to 127. The highest bit is ignored. 
	unsigned char pid; 	
	
	// Payload length, 0 to 255 bytes. 
	unsigned char len; 
	
	// Payload capacity, 0 to 255 bytes.
	unsigned char capacity; 
	
	// Pointer to allocated payload array. 
	unsigned char * payload; 
} Packet_t; 

// Procedure: partial constructor (does not populate all elements)
extern void Packet_New(Packet_t * packet, unsigned char * payload, int capacity); 


#define PACKET_CONSTRUCT_SUCCESS 0
#define PACKET_CONSTRUCT_FAIL -1

// PacketInBatteryInfo
#define PacketInBatteryInfo_ID 0x0F
#define PacketInBatteryInfo_Length 4
extern int PacketInBatteryInfo(
	Packet_t * packet, 
	unsigned short voltage, 
	unsigned char percentage, 
	unsigned char state
); 

// PacketInNewOpticalSample
#define PacketInNewOpticalSample_ID 0x20
#define PacketInNewOpticalSample_Length 6
extern int PacketInNewOpticalSample(
	Packet_t * packet, 
	unsigned int seconds, 
	const unsigned char * meas2
); 

// PacketInNewOpticalEstimation
#define PacketInNewOpticalEstimation_ID 0x21
#define PacketInNewOpticalEstimation_Length 7
extern int PacketInNewOpticalEstimation(
	Packet_t * packet, 
	unsigned char interval, 
	unsigned int sample, 
	const unsigned char * meas2
); 

// PacketInSyncInfo
#define PacketInSyncInfo_ID 0x22
#define PacketInSyncInfo_Length 10
extern int PacketInSyncInfo(
	Packet_t * packet, 
	unsigned int sampleStart, 
	unsigned int sampleCount, 
	unsigned char secondCounter, 
	unsigned char interval
); 

// PacketInSyncData
#define PacketInSyncData_ID 0x23
#define PacketInSyncData_BaseLength 5
extern int PacketInSyncData(
	Packet_t * packet, 
	unsigned int sampleStart, 
	unsigned char sampleCount
); 
extern void PacketInSyncData_WriteSample(
	Packet_t * packet, 
	unsigned int index, 
	const unsigned char * meas2
); 

// PacketOutRequestSyncInfo
#define PacketOutRequestSyncInfo_ID 0x22
#define PacketOutRequestSyncInfo_Length 0

// PacketOutRequestSyncData
#define PacketOutRequestSyncData_ID 0x23
#define PacketOutRequestSyncData_Length 5
#define MAX_SYNC_REQUEST_COUNT 125
extern unsigned int PacketOutRequestSyncData_ReadStartSample(const Packet_t * packet); 
extern unsigned char PacketOutRequestSyncData_ReadCount(const Packet_t * packet); 

#endif
