#ifndef BLETHREAD_H__
#define BLETHREAD_H__

#include "liblwt.h"
#include "looper.h"

#include "libstream.h"
#include "libpacket.h"

typedef struct {
	LWT_t lwt; 
	unsigned int stage; 
	unsigned int flags; 
	struct {
		Stream_t tx; 
		Stream_t rx; 
	} streams; 
} BleThread_State_t; 

extern BleThread_State_t BleThread_Stat; 

extern void BleThread_Init(void); 

extern void BleThread_Start(void); 

extern void BleThread_TxRawMessage(const char * string); 
extern void BleThread_TxMessage(const char * string); 
extern void BleThread_TxPacket(const Packet_t * packet); 

extern int BleThread_IsConnected(void); 

// Outside of thread context
__weak void BleThread_HandleConnectionFlow(int isConnected); 

// Inside of thread context, where local variables are stacked
__weak void BleThread_HandlePacket(const Packet_t * packet); 

#endif
