#include <stm32g071xx.h>

#include "looper.h"

#include "libsys.h"
#include "libi2c.h"
#include "libuartble.h"

#include "libprotocol.h"

#include "lwtdaq.h"

void callback(void); 
void callback1(void); 

void callback(void) {
	for(int i = 0; i < 3; i++)
		Protocol_TxMessage("WAKEUPWAKEUP"); 
	MainLooper_SubmitDelayed(Protocol_RxProcessingDone, 700); 
}

void callback1(void) {
	LED_Green_On(); 
	LWTDAQ_Trigger(); 
	MainLooper_SubmitDelayed(callback1, 1000); 
}

void LWTDAQ_Callback(void) {
	Packet_t packet; 
	unsigned char payload[6]; 
	packet.dir = 1; 
	packet.pid = 0x20; // PacketInNewSample
	packet.len = 6; 
	packet.payload = payload; 
	unsigned int uv = LWTDAQ.meas.uv; 
	unsigned int vis = LWTDAQ.meas.vis; 
	payload[0] = vis >> 16; 
	payload[1] = vis >> 8; 
	payload[2] = vis; 
	payload[3] = uv >> 16; 
	payload[4] = uv >> 8; 
	payload[5] = uv; 
	Protocol_TxPacket(&packet); 
	LED_Green_Off(); 
}

void Protocol_OnRxPacket_Autoend(const Packet_t * packet) {
	__nop();
	if(!LWTDAQ.busy) {
		LED_Green_On(); 
		LWTDAQ_Trigger(); 
	}
}

void Protocol_OnRxPacket(const Packet_t * packet) {
	Protocol_OnRxPacket_Autoend(packet); 
	Protocol_RxProcessingDone(); 
}

int main(void) {
	Sys_Init(); 
	LED_Blue_On(); 
	while(!Sys_LSEReady()); 
	LED_Blue_Off(); 
	
	Task_Init(); 
	MainLooper_Init(); 
	
	I2C_HWInit(); 
	UARTBLE_Init(); 
	
	LWTDAQ_Init(); 
	
	MainLooper_SubmitDelayed(callback, 100); 
	MainLooper_Entry(); 
	while(1); 
}


