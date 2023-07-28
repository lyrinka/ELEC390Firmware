#ifndef LIBCODEC_H__
#define LIBCODEC_H__

#include "libpacket.h"

typedef struct {
	unsigned int capacity; 
	unsigned int length; 
	unsigned char * buffer; 
} Codec_Buffer_t; 

// Packet encoder
#define PACKET_ENCODE_SUCCESS 0
#define PACKET_ENCODE_OUTPUT_TOO_SMALL 1
extern int Packet_Encode(const Packet_t * obj, Codec_Buffer_t * buffer); 

// Packet decoder
#define PACKET_DECODE_SUCCESS 0
#define PACKET_DECODE_CORRUPTED 1
#define PACKET_DECODE_INPUT_TOO_SHORT 2
#define PACKET_DECODE_OUTPUT_TOO_SMALL 3
extern int Packet_Decode(const Codec_Buffer_t * buffer, Packet_t * obj); 

#endif
