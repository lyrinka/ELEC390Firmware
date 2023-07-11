#ifndef LIBPACKET_H__
#define LIBPACKET_H__

typedef struct {
	unsigned char dir; 
	unsigned char pid; 	
	unsigned char len; 
	unsigned char * payload; 
} Packet_t; 


#define PACKET_ENCODE_SUCCESS 0
#define PACKET_ENCODE_CONSUMER_FULL 1
extern int Packet_Encode(const Packet_t * obj, int (*consumer)(void * context, unsigned char data), void * context); 

#define PACKET_DECODE_SUCCESS 0
#define PACKET_DECODE_PRODUCER_EMPTY 1
#define PACKET_DECODE_CORRUPTED 2
extern int Packet_Decode(Packet_t * obj, int (*producer)(void * context), void * context); 

#endif
