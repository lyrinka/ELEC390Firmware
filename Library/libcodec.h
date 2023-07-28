#ifndef LIBCODEC_H__
#define LIBCODEC_H__

#include "libpacket.h"

typedef struct {
	unsigned int capacity; 
	unsigned int length; 
	unsigned char * buffer; 
} Codec_Buffer_t; 

// Packet encoder
#define CODEC_ENCODE_SUCCESS 0
#define CODEC_ENCODE_OUTPUT_TOO_SMALL 1
extern int Codec_Encode(const Packet_t * obj, Codec_Buffer_t * buffer); 

// Packet decoder
#define CODEC_DECODE_SUCCESS 0
#define CODEC_DECODE_CORRUPTED 1
#define CODEC_DECODE_INPUT_TOO_SHORT 2
#define CODEC_DECODE_OUTPUT_TOO_SMALL 3
extern int Codec_Decode(const Codec_Buffer_t * buffer, Packet_t * obj); 

#endif
