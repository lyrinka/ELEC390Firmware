#include <stm32g071xx.h>

#include "looper.h"

#include "libsys.h"
#include "libi2c.h"
#include "libuartble.h"

#include "libpackets.h"
#include "libprotocol.h"

#include "lwtdaq.h"

int strcmprefix(const char * str1, unsigned int length, const char * str2) {
	for(int i = 0; i < 32; i++) {
		if(i >= length) return 1; 
		char ch1 = str1[i]; 
		char ch2 = str2[i]; 
		if(ch1 == 0) return 0; 
		if(ch2 == 0) return 1; 
		if(ch1 != ch2) return 0; 
	}
	return 1; // max length reached
}

int connected = 0; 

void Initialization(void) {
	connected = 0; 
	Protocol_RxProcessingDone(); 
}

void Protocol_OnRxMessage(const char * string, unsigned int length) {
	if(strcmprefix(string, length, "CONNECTED")) {
		Protocol_TxMessage("WAKEUPWAKEUPWAKEUPWAKEUPWAKEUP"); 
	}
	else if(strcmprefix(string, length, "WAKE UP")) {
		connected = 1; 
		LWTDAQ.ledMode = LWTDAQ_LEDMODE_CONNECTED; 
	}
	else if(strcmprefix(string, length, "DISCONNECTED")) {
		connected = 0; 
		LWTDAQ.ledMode = LWTDAQ_LEDMODE_DISCONNECTED; 
		Protocol_TxMessage("+++AT+SLEEP=0,1,1"); 
	}
	Protocol_RxProcessingDone(); 
}


void Protocol_OnRxPacket(const Packet_t * packet) {
	Protocol_RxProcessingDone(); 
}

void LWTDAQ_Callback(void) {
	if(!connected) return; 
	Packet_t packet; 
	unsigned char payload[PacketInNewSample_Length]; 
	PacketInNewSample(&packet, payload, LWTDAQ.minutes, LWTDAQ.seconds, LWTDAQ.compressedMeasurement); 
	Protocol_TxPacket(&packet); 
}

int main2(void) {
	// HW components
	Sys_Init(); 
	LED_Blue_On(); 
	while(!Sys_LSEReady()); 
	LED_Blue_Off(); 
	// OS
	Task_Init(); 
	MainLooper_Init(); 
	// SW components
	I2C_HWInit(); 
	UARTBLE_Init(); 
	LWTDAQ_Init(); 
	// Entry
	MainLooper_Submit(Initialization); 
	LWTDAQ_Start(); 
	MainLooper_Entry(); 
	while(1); 
}


