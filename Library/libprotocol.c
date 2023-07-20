#include "libprotocol.h"

#include "looper.h"

#include "libuartble.h"

struct {
	unsigned int txpackets; 
	unsigned int txmessages; 
	unsigned int txfailures; 
	unsigned int rxpackets; 
	unsigned int rxmessages; 
	unsigned int rxfailures; 
} Protocol_Profiling; 

Packet_t Protocol_DecodingPacket; 

// TODO: confirm that maximum size is 325
#define PROTOCOL_ENCODINGBUFFER_SIZE 325
unsigned char Protocol_EncodingBuffer[PROTOCOL_ENCODINGBUFFER_SIZE]; 

#define PROTOCOL_DECODINGBUFFER_SIZE 255
unsigned char Protocol_DecodingBuffer[PROTOCOL_DECODINGBUFFER_SIZE]; 

// Tx
int Protocol_TxPacket(const Packet_t * packet) {
	unsigned int length = 0; 
	int status = Packet_Encode(packet, Protocol_EncodingBuffer, PROTOCOL_ENCODINGBUFFER_SIZE, &length); 
	if(status != PACKET_ENCODE_SUCCESS) {
		// TODO: how do we handle this? This should not happen. 
		return PROTOCOL_TX_INTERNAL_ERR; 
	}
	status = UARTBLE_WriteWithPrefix('#', Protocol_EncodingBuffer, length); 
	if(status == UARTBLE_WRITE_FAIL) {
		Protocol_Profiling.txfailures++; 
		return PROTOCOL_TX_RETRY; 
	}
	Protocol_Profiling.txpackets++; 
	return PROTOCOL_TX_SUCCESS; 
}

int Protocol_TxMessage(const char * string) {
	unsigned int index = 0; 
	while(string[index]) index++; 
	int status = UARTBLE_Write((const unsigned char *)string, index); 
	if(status == UARTBLE_WRITE_FAIL) {
		Protocol_Profiling.txfailures++; 
		return PROTOCOL_TX_RETRY; 
	}
	Protocol_Profiling.txmessages++; 
	return PROTOCOL_TX_SUCCESS; 
}

// Rx
__weak void Protocol_OnRxPacket(const Packet_t * packet) {
	Protocol_RxProcessingDone(); 
}

__weak void Protocol_OnRxBrokenPacket(const char * input, unsigned int length) {
	Protocol_RxProcessingDone(); 
}

__weak void Protocol_OnRxMessage(const char * input, unsigned int length) {
	Protocol_RxProcessingDone(); 
}

void Protocol_OnRxPacketInternal(const unsigned char * input, unsigned int length) {
	Protocol_DecodingPacket.payload = Protocol_DecodingBuffer; 
	int status = Packet_Decode(input, length, &Protocol_DecodingPacket, PROTOCOL_DECODINGBUFFER_SIZE); 
	if(status == PACKET_DECODE_SUCCESS) {
		Protocol_Profiling.rxpackets++; 
		Protocol_OnRxPacket(&Protocol_DecodingPacket); 
	}
	else {
		Protocol_Profiling.rxfailures++; 
		Protocol_OnRxBrokenPacket((const char *)input, length); 
	}
}

void UARTBLE_RxLineCallback(void) {
	unsigned char * line = UARTBLE.lineParser.buffer; 
	unsigned int size = UARTBLE.lineParser.size; 
	if(line[0] == '#') 
		Protocol_OnRxPacketInternal(line + 1, size - 1); 
	else {
		Protocol_Profiling.rxmessages++; 
		Protocol_OnRxMessage((const char *)line, size); 
	}
}

void Protocol_RxProcessingDone(void) {
	UARTBLE_RxLineRelease(); 
}
