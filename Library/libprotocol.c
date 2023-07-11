#include <stm32g071xx.h>

#include "libprotocol.h"

#define critEnter() unsigned int __X_IE = __get_PRIMASK(); __disable_irq()
#define critExit() __set_PRIMASK(__X_IE)

#define RXFSM_STATE_READING 0
#define RXFSM_STATE_BREAK 1
#define RXFSM_STATE_OVERFLOW 2

unsigned char TxBuffer[TXBUFFER_CAPACITY]; 
unsigned char RxBuffer[RXBUFFER_CAPACITY]; 
unsigned char RxLineBuf[RXLINEBUF_CAPACITY]; 

Protocol_t ProtocolManager; 

void Protocol_Init(void) {
	Fifo_Init(&ProtocolManager.txFifo, TxBuffer, TXBUFFER_CAPACITY); 
	Fifo_Init(&ProtocolManager.rxFifo, RxBuffer, RXBUFFER_CAPACITY); 
	ProtocolManager.linebuf.buffer = RxLineBuf; 
	ProtocolManager.linebuf.capacity = RXLINEBUF_CAPACITY; 
	ProtocolManager.linebuf.index = 0; 
	ProtocolManager.fsm.state = RXFSM_STATE_READING; 
}

int Protocol_GetTx(void) {
	return Fifo_Get(&ProtocolManager.txFifo); 
}

int Protocol_TxString(const char * str) {
	int index = 0; 
	for(;;) {
		char ch = str[index]; 
		if(ch == 0) break; 
		if(Fifo_Put(&ProtocolManager.txFifo, ch) != FIFO_SUCCESS) return -1; 
		index++; 
	}
	if(Fifo_Put(&ProtocolManager.txFifo, 0x0D) != FIFO_SUCCESS) return -1; 
	if(Fifo_Put(&ProtocolManager.txFifo, 0x0A) != FIFO_SUCCESS) return -1; 
	return 0; 
}

static int txConsumer(void * context, unsigned char data) {
	int status = Fifo_Put(&ProtocolManager.txFifo, data); 
	return (status == FIFO_SUCCESS) ? 0 : 1; 
}
int Protocol_TxPacket(const Packet_t * packet) {
	if(Fifo_Put(&ProtocolManager.txFifo, '#' ) != FIFO_SUCCESS) return -1; 
	if(Packet_Encode(packet, txConsumer, 0) != PACKET_ENCODE_SUCCESS) return -1; 
	if(Fifo_Put(&ProtocolManager.txFifo, 0x0D) != FIFO_SUCCESS) return -1; 
	if(Fifo_Put(&ProtocolManager.txFifo, 0x0A) != FIFO_SUCCESS) return -1; 
	return 0; 
}

int Protocol_PutRx(unsigned char data) {
	switch(ProtocolManager.state) {
		case RXFSM_STATE_READING: 
		default: {
			if(data >= ' ' && data <= '~') {
				int index = ProtocolManager.linebuf.index; 
				if(index >= ProtocolManager.linebuf.capacity) {
					ProtocolManager.state = RXFSM_STATE_OVERFLOW; 
					return -1; 
				}
				ProtocolManager.linebuf.buffer[index] = data; 
				ProtocolManager.linebuf.index = index + 1; 
				return 0; 
			}
			if(data == 0x0D || data == 0x0A) {
				ProtocolManager.state = RXFSM_STATE_BREAK; 
				int length = ProtocolManager.linebuf.index; 
				if(length > ProtocolManager.rxFifo.count) 
					return -1; 
				for(int i = 0; i < length; i++) 
					Fifo_Put(&ProtocolManager.rxFifo, ProtocolManager.linebuf.buffer[i]); 
				return 0; 
			}
			
		}
		case RXFSM_STATE_BREAK: {
			
			
			
		}
		case RXFSM_STATE_OVERFLOW: {
			
			
			
		}
	}
	
	
	
}
