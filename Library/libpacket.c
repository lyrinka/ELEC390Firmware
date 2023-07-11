#include "libpacket.h"

static unsigned int fastmod(unsigned int * pN) {
	unsigned int N = *pN; 
	unsigned int d = ((unsigned long long)N * 1616385542) >> 32; 
	unsigned int Q = (((N - d) >> 1) + d) >> 6; 
	*pN = Q; 
	unsigned int R = N - Q * 93; 
	return R; 
}

static void base93Encode(const unsigned char * input4, unsigned char * output5) {
	unsigned int N, R; 
	N	 = input4[3]; N <<= 8; 
	N |= input4[2]; N <<= 8; 
	N |= input4[1]; N <<= 8; 
	N |= input4[0]; 
	R = fastmod(&N) + '!'; if(R >= '+') R++; output5[0] = R; 
	R = fastmod(&N) + '!'; if(R >= '+') R++; output5[1] = R; 
	R = fastmod(&N) + '!'; if(R >= '+') R++; output5[2] = R; 
	R = fastmod(&N) + '!'; if(R >= '+') R++; output5[3] = R; 
	N += '!'; 						 if(N >= '+') N++; output5[4] = N; 
}

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

static void crc8Accept(unsigned char * pcrc, unsigned char input) {
	unsigned int crc = *pcrc; 
	crc ^= input; 
	for(int i = 0; i < 8; i++) {
		crc <<= 1; 
		if(crc & 0x100) crc ^= 0x1D; 		
	}
	*pcrc = crc; 
}


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
