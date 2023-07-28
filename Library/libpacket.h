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

#define PacketInNewSample_ID 0x20
#define PacketInNewSample_Length 6
extern int PacketInNewSample(
	Packet_t * packet, 
	unsigned int seconds, 
	const unsigned char * meas2
); 

#endif
