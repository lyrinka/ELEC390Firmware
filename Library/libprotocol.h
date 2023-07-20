#ifndef LIBPROTOCOL_H__
#define LIBPROTOCOL_H__

#include "libpacket.h"

extern Packet_t Protocol_DecodingPacket; 

#define PROTOCOL_TX_SUCCESS 0
#define PROTOCOL_TX_RETRY -1
#define PROTOCOL_TX_INTERNAL_ERR -2
extern int Protocol_TxPacket(const Packet_t * packet); 
extern int Protocol_TxMessage(const char * string); 
extern int Protocol_TxMessageNoCRLF(const char * string); 

__weak void Protocol_OnRxPacket(const Packet_t * packet); 
__weak void Protocol_OnRxBrokenPacket(const char * input, unsigned int length); 
__weak void Protocol_OnRxMessage(const char * input, unsigned int length); 
extern void Protocol_RxProcessingDone(void); 

#endif
