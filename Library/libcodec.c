#include "libcodec.h"

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
	N += '!'; 						 	 if(N >= '+') N++; output5[4] = N; 
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

int Packet_Encode(const Packet_t * packet, Codec_Buffer_t * buffer) {
	int inputPayloadLength = packet->len; 
	
	int outputBufferWriterIndex = 0; 
	int inputPayloadReaderIndex = -2; 
	
	unsigned char crc = 0xFF; 
	char endOfPacket = 0; 
	
	unsigned char internalBuffer[5]; 
	
	for(;;) {
		for(int i = 0; i < 4; i++) {
			unsigned char data; 
			if(endOfPacket) {
				data = 0; 
			}
			else if(inputPayloadReaderIndex == -2) {
				data = (packet->pid & 0x7F) | (packet->dir ? 0x80 : 0x00); 
				inputPayloadReaderIndex = -1; 
			}
			else if(inputPayloadReaderIndex == -1) {
				data = inputPayloadLength; 
				inputPayloadReaderIndex = 0; 
			}
			else if(inputPayloadReaderIndex < inputPayloadLength) {
				data = packet->payload[inputPayloadReaderIndex++]; 
			}
			else {
				data = crc ^ 0xFF; 
				endOfPacket = 1; 
			}
			if(!endOfPacket) {
				crc8Accept(&crc, data); 
			}
			internalBuffer[i] = data; 
		}
		base93Encode(internalBuffer, internalBuffer); 
		for(int i = 0; i < 5; i++) {
			if(outputBufferWriterIndex >= buffer->capacity) 
				return PACKET_ENCODE_OUTPUT_TOO_SMALL; 
			buffer->buffer[outputBufferWriterIndex++] = internalBuffer[i]; 
		}
		if(endOfPacket) {
			buffer->length = outputBufferWriterIndex; 
			return PACKET_ENCODE_SUCCESS; 
		}
	}
}

int Packet_Decode(const Codec_Buffer_t * buffer, Packet_t * packet) {
	int inputBufferReaderIndex = 0; 
	int outputPayloadWriterIndex = -2; 
	
	unsigned char crc = 0xFF; 
	unsigned int payloadLength; 
	
	unsigned char internalBuffer[5]; 
	
	for(;;) {
		for(int i = 0; i < 5; i++) {
			if(inputBufferReaderIndex >= buffer->length) 
				return PACKET_DECODE_INPUT_TOO_SHORT; 
			internalBuffer[i] = buffer->buffer[inputBufferReaderIndex++]; 
		}
		base93Decode(internalBuffer, internalBuffer); 
		for(int i = 0; i < 4; i++) {
			unsigned char data = internalBuffer[i]; 
			if(outputPayloadWriterIndex == -2) {
				packet->dir = data & 0x80 ? 1 : 0; 
				packet->pid = data & 0x7F; 
				outputPayloadWriterIndex = -1; 				
			}
			else if(outputPayloadWriterIndex == -1) {
				payloadLength = data; 
				packet->len = data; 
				if(payloadLength >= packet->capacity) 
					return PACKET_DECODE_OUTPUT_TOO_SMALL; 
				outputPayloadWriterIndex = 0; 
			}
			else if(outputPayloadWriterIndex < payloadLength) {
				packet->payload[outputPayloadWriterIndex++] = data; 
			}
			else {
				if(data != (crc ^ 0xFF)) 
					return PACKET_DECODE_CORRUPTED; 
				return PACKET_DECODE_SUCCESS; 
			}
			crc8Accept(&crc, data); 
		}
	}
}
