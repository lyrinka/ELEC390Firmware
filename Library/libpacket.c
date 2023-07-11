#include "libpacket.h"

// Fastmod93 takes the pointer to a number N and performs the following integer operations: 
// R := N % 93
// N := N / 93
// N is updated via reference and R is returned. 
static unsigned int fastmod93(unsigned int * pN) {
	unsigned int N = *pN; 
	unsigned int d = ((unsigned long long)N * 1616385542) >> 32; 
	unsigned int Q = (((N - d) >> 1) + d) >> 6; 
	*pN = Q; 
	unsigned int R = N - Q * 93; 
	return R; 
}

// Base93 Encoder takes an array of 4 bytes and encodes as 5 characters. 
static void base93Encode(const unsigned char * input4, unsigned char * output5) {
	unsigned int N, R; 
	N	 = input4[3]; N <<= 8; 
	N |= input4[2]; N <<= 8; 
	N |= input4[1]; N <<= 8; 
	N |= input4[0]; 
	R = fastmod93(&N) + '!'; if(R >= '+') R++; output5[0] = R; 
	R = fastmod93(&N) + '!'; if(R >= '+') R++; output5[1] = R; 
	R = fastmod93(&N) + '!'; if(R >= '+') R++; output5[2] = R; 
	R = fastmod93(&N) + '!'; if(R >= '+') R++; output5[3] = R; 
	N += '!'; 						 if(N >= '+') N++; output5[4] = N; 
}

// Base93 Decoder takes an array of 5 characters and decodes as 4 bytes. 
// This method does not attempt to check input characters against valid range. 
static void base93Decode(const unsigned char * input5, unsigned char * output4) {
	unsigned int R, N; 
	N = input5[4]; if(N >= '+') N--; N -= '!'; 
	R = input5[3]; if(R >= '+') R--; N = (R - '!') + N * 93; 
	R = input5[2]; if(R >= '+') R--; N = (R - '!') + N * 93; 
	R = input5[1]; if(R >= '+') R--; N = (R - '!') + N * 93; 
	R = input5[0]; if(R >= '+') R--; N = (R - '!') + N * 93; 
	output4[0] = N; N >>= 8; 
	output4[1] = N; N >>= 8; 
	output4[2] = N; N >>= 8; 
	output4[3] = N; 
}

// CRC8 consumer merges input to a 8-bit CRC register, passed by reference. 
// The CRC polynomial implemented is 0x1D (SAE J1850). 
// As per J1850, the initial value shall be 0xFF and the result CRC value shall be inverted. 
// Reference: http://www.sunshine2k.de/coding/javascript/crc/crc_js.html
static void crc8Accept(unsigned char * pcrc, unsigned char input) {
	unsigned int crc = *pcrc; 
	crc ^= input; 
	for(int i = 0; i < 8; i++) {
		crc <<= 1; 
		if(crc & 0x100) crc ^= 0x1D; 		
	}
	*pcrc = crc; 
}

// Packet encode method reads a packet and passes encoded charaters to a byte-oriented consumer. 
// The consumer shall return 0 on success and 1 when the consumer cannot accept input data. 
int Packet_Encode(const Packet_t * obj, int (*consumer)(void * context, unsigned char data), void * context) {
	int payloadLength = obj->len; 
	int packetReaderIndex = -2; 
	unsigned char packetCRC = 0xFF; 
	int endOfPacket = 0; 
	unsigned char codecBuffer[5]; 
	
	for(;;) {
		for(int i = 0; i < 4; i++) {
			unsigned char data; 
			if(endOfPacket) {
				data = 0; 
			}
			else {
				if(packetReaderIndex == -2) {
					data = (obj->pid & 0x7F) | (obj->dir ? 0x80 : 0x00); 
					packetReaderIndex = -1; 
				}
				else if(packetReaderIndex == -1) {
					data = payloadLength; 
					packetReaderIndex = 0; 
				}
				else if(packetReaderIndex < payloadLength) {
					data = obj->payload[packetReaderIndex]; 
					packetReaderIndex++; 
				}
				else {
					data = packetCRC ^ 0xFF; 
					endOfPacket = 1; 
				}
				if(!endOfPacket) crc8Accept(&packetCRC, data); 
			}
			codecBuffer[i] = data; 
		}
		base93Encode(codecBuffer, codecBuffer); 
		for(int i = 0; i < 5; i++) {
			if(consumer(context, codecBuffer[i])) 
				return PACKET_ENCODE_CONSUMER_FULL; 
		}
		if(endOfPacket) return PACKET_ENCODE_SUCCESS; 
	}
}

// Packet decode method reads from a byte-oriented producer and attempts to parse a packet. 
// The producer shall return one byte on success and -1 when the producer cannot produce output data. 
// TODO: array out of bounds check
int Packet_Decode(Packet_t * obj, int (*producer)(void * context), void * context) {
	int payloadLength; 
	int packetWriterIndex = -2; 
	unsigned char packetCRC = 0xFF; 
	unsigned char codecBuffer[5]; 
	
	for(;;) {
		for(int i = 0; i < 5; i++) {
			int data = producer(context); 
			if(data < 0) return PACKET_DECODE_PRODUCER_EMPTY; 
			codecBuffer[i] = data; 
		}
		base93Decode(codecBuffer, codecBuffer); 
		for(int i = 0; i < 4; i++) {
			unsigned char data = codecBuffer[i]; 
			if(packetWriterIndex == -2) {
				obj->dir = data & 0x80 ? 1 : 00; 
				obj->pid = data & 0x7F; 
				packetWriterIndex = -1; 
			}
			else if(packetWriterIndex == -1) {
				payloadLength = data; 
				obj->len = data; 
				packetWriterIndex = 0; 
			}
			else if(packetWriterIndex < payloadLength) {
				obj->payload[packetWriterIndex] = data; 
				packetWriterIndex++; 
			}
			else {
				if(data != (packetCRC ^ 0xFF)) return PACKET_DECODE_CORRUPTED; 
				else return PACKET_DECODE_SUCCESS; 
			}
			crc8Accept(&packetCRC, data); 
		}
	}
}
