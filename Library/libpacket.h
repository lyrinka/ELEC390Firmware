#ifndef LIBPACKET_H__
#define LIBPACKET_H__

// Represents a packet. 
typedef struct {
	// Direction of the packet with respect to the remote device. 
	// Zero implies OUT packet (host to MCU), non-zero implies IN packet (MCU to host). 
	unsigned char dir; 
	
	// Packet identifier, valid 0 to 127. The highest bit is ignored. 
	unsigned char pid; 	
	
	// Payload length, 0 to 255 bytes. 
	unsigned char len; 
	
	// Pointer to allocated payload array. 
	unsigned char * payload; 
} Packet_t; 


// Packet encoder
#define PACKET_ENCODE_SUCCESS 0
#define PACKET_ENCODE_CONSUMER_FULL 1
extern int Packet_Encode(const Packet_t * obj, int (*consumer)(void * context, unsigned char data), void * context); 

// Packet decoder
#define PACKET_DECODE_SUCCESS 0
#define PACKET_DECODE_PRODUCER_EMPTY 1
#define PACKET_DECODE_CORRUPTED 2
#define PACKET_DECODE_BUFFER_TOO_SMALL 3
extern int Packet_Decode(Packet_t * obj, int buflen, int (*producer)(void * context), void * context); 

#endif
