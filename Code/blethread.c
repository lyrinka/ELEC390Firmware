#include "blethread.h"

#include "libcodec.h"

#include "libsys.h"
#include "libuart.h"

BleThread_State_t BleThread_State; 

/* -------- LWT instantiation -------- */
#define BLETHREAD_STACK_SIZE 512
unsigned char BleThread_Stack[BLETHREAD_STACK_SIZE];  

void BleThread_Init(void) {
	// Data structure
	BleThread_State.totalTxPacket = 0; 
	BleThread_State.totalRxPacket = 0; 
	BleThread_State.totalTxMessage = 0; 
	BleThread_State.totalRxMessage = 0; 
	// UART middleware
	void BleThread_SerialInit(void); 
	BleThread_SerialInit(); 
	
	// Application FSM
	void BleThread_ConnectionFlowInit(void); 
	BleThread_ConnectionFlowInit(); 
	
	// LWT
	void BleThread_Entry(void); 
	LWT_New(&BleThread_State.lwt, BleThread_Stack, BLETHREAD_STACK_SIZE, BleThread_Entry); 
}

void BleThread_Start(void) {
	MainLooper_Submit2(LWT_Dispatch1, &BleThread_State.lwt, 0); 
}


/* -------- Event system -------- */
unsigned char BleThread_BlockingReadBytes(void) {
	for(;;) {
		unsigned char data; 
		int status = Stream_Read(&BleThread_State.streams.rx, &data); 
		if(status == STREAM_READ_SUCCESS) return data; 
		LWT_Yield1(); 
	}
}


/* -------- UART middleware -------- */
#define BLETHREAD_TXBUFFER_SIZE 1024
#define BLETHREAD_RXBUFFER_SIZE 1024
unsigned char BleThread_TxStream_Buffer[BLETHREAD_TXBUFFER_SIZE]; 
unsigned char BleThread_RxStream_Buffer[BLETHREAD_RXBUFFER_SIZE]; 

void BleThread_SerialInit(void) {
	Stream_Init(&BleThread_State.streams.tx, BleThread_TxStream_Buffer, BLETHREAD_TXBUFFER_SIZE); 
	Stream_Init(&BleThread_State.streams.rx, BleThread_RxStream_Buffer, BLETHREAD_RXBUFFER_SIZE); 
	UART_Init(); 
}

// External weak function linkage -> libuart.c
// IRQ context!!
void UART_RxFullHandler(unsigned char data, int overrun) {
	Stream_Write(&BleThread_State.streams.rx, data); 
	MainLooper_Submit2(LWT_Dispatch1, &BleThread_State.lwt, 0); 
}

// External weak function linkage -> libuart.c
// IRQ context!!
int UART_TxEmptyHandler(unsigned char * data) {
	int status = Stream_Read(&BleThread_State.streams.tx, data); 
	if(status != STREAM_READ_SUCCESS) 
		return UART_TXHANDLER_NODATA; 
	return UART_TXHANDLER_SUCCESS; 
}


/* -------- Protocol middleware -------- */
#define getch() BleThread_BlockingReadBytes()
#define isPrintable(ch) ((ch) >= ' ' && (ch) <= '~')
int BleThread_ReadLine(unsigned char * buffer, unsigned int capacity) {
	for(;;) {
		for(int i = 0; i < capacity; i++) {
			unsigned char ch = getch(); 
			if(isPrintable(ch)) {
				buffer[i] = ch; 
				continue; 
			}
			if(i == 0) {
				i--; 
				continue; 
			}
			return i; 
		}
		while(!isPrintable(getch())); 
	}
}

int BleThread_ParsePacket(unsigned char * buffer, unsigned int length, Packet_t * packet) {
	if(buffer[0] != '#') return -1; 
	Codec_Buffer_t bufferStruct; 
	bufferStruct.capacity = length - 1; 
	bufferStruct.length = length - 1; 
	bufferStruct.buffer = buffer + 1; 
	int status = Codec_Decode(&bufferStruct, packet); 
	if(status != CODEC_DECODE_SUCCESS) return -2; 
	return 0; 
}


/* -------- Thread body -------- */
// Line buffer is used during line parsing, message rx/tx, rx packet payload & encodings, and tx packet encodings. 
// Line buffer shall not be used to store tx packet payload.
#define BLETHREAD_LINEBUFFER_SIZE 350
unsigned char BleThread_RxLineBuffer[BLETHREAD_LINEBUFFER_SIZE]; 
unsigned char BleThread_TxLineBuffer[BLETHREAD_LINEBUFFER_SIZE]; 

void BleThread_TxRawMessage(const char * string) {
	for(int i = 0;; i++) {
		char ch = string[i]; 
		if(!ch) break; 
		int status = Stream_Write(&BleThread_State.streams.tx, ch); 
		UART_TriggerTx(); 
		if(status != STREAM_WRITE_SUCCESS) break;
	}
}

void BleThread_TxMessage(const char * string) {
	BleThread_TxRawMessage(string); 
	BleThread_TxRawMessage("\r\n"); 
	BleThread_State.totalTxMessage++; 
}

void BleThread_TxPacket(const Packet_t * packet) {
	Codec_Buffer_t bufferStruct; 
	bufferStruct.capacity = BLETHREAD_LINEBUFFER_SIZE - 1; 
	bufferStruct.buffer = BleThread_TxLineBuffer; 
	int status = Codec_Encode(packet, &bufferStruct); 
	if(status != CODEC_ENCODE_SUCCESS) return; 
	bufferStruct.buffer[bufferStruct.length] = 0; 
	BleThread_TxRawMessage("#"); 
	BleThread_TxMessage((char *)bufferStruct.buffer); 
	BleThread_State.totalTxPacket++; 
}

void BleThread_Entry(void) {
	void BleThread_InternalHandleMessage(char * string, unsigned int length); 
	void BleThread_InternalHandlePacket(Packet_t * packet); 
	
	Packet_t packet; 
	packet.capacity = BLETHREAD_LINEBUFFER_SIZE > 255 ? 255 : BLETHREAD_LINEBUFFER_SIZE; 
	packet.payload = BleThread_RxLineBuffer; 
	for(;;) {
		int length = BleThread_ReadLine(BleThread_RxLineBuffer, BLETHREAD_LINEBUFFER_SIZE); 
		int status = BleThread_ParsePacket(BleThread_RxLineBuffer, length, &packet); 
		if(status) {
			BleThread_State.totalRxMessage++; 
			BleThread_InternalHandleMessage((char *)BleThread_RxLineBuffer, length); 
		}
		else {
			BleThread_State.totalRxPacket++; 
			BleThread_InternalHandlePacket(&packet); 
		}
	}
}

/* -------- Connection handling -------- */
static int strcmprefix(const char * str1, unsigned int length, const char * str2) {
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
#define strcmpx(str) strcmprefix(string, length, str)

#define BLE_CONNECTIon_GRACEPERIOD	1000

#define BLE_STAGE_DISCONNECTED						0
#define BLE_STAGE_CONNECTION_GRACEPERIOD	1
#define BLE_STAGE_ESTABLISHED							2

void BleThread_ConnectionFlowInit(void) {
	BleThread_State.stage = BLE_STAGE_DISCONNECTED; 
}

// Outside of thread context
void BleThread_Lambda1(void) {
	if(BleThread_State.stage != BLE_STAGE_CONNECTION_GRACEPERIOD) return; 
	BleThread_State.stage = BLE_STAGE_ESTABLISHED; 
	BleThread_HandleConnectionFlow(1); 
}

// Outside of thread context
void BleThread_Lambda2(void) {
	BleThread_HandleConnectionFlow(0); 
}

void BleThread_InternalHandleMessage(char * string, unsigned int length) {
	if(strcmpx("CONNECTED")) {
		BleThread_TxMessage("WAKEUPWAKEUPWAKEUPWAKEUPWAKEUP"); 
		if(BleThread_State.stage == BLE_STAGE_DISCONNECTED) {
			BleThread_State.stage = BLE_STAGE_CONNECTION_GRACEPERIOD; 
			MainLooper_SubmitDelayed(BleThread_Lambda1, BLE_CONNECTIon_GRACEPERIOD); 
		}
	}
	else if(strcmpx("DISCONNECTED")) {
		BleThread_TxMessage("+++AT+SLEEP=0,1,1"); 
		if(BleThread_State.stage == BLE_STAGE_ESTABLISHED) {
			MainLooper_Submit(BleThread_Lambda2); 
		}
		BleThread_State.stage = BLE_STAGE_DISCONNECTED; 
	}
}

void BleThread_InternalHandlePacket(Packet_t * packet) {
	if(BleThread_State.stage != BLE_STAGE_ESTABLISHED) return; 
	BleThread_HandlePacket(packet); 
}

int BleThread_IsConnected(void) {
	return BleThread_State.stage == BLE_STAGE_ESTABLISHED; 
}

__weak void BleThread_HandleConnectionFlow(int isConnected) {
	return; 
}

__weak void BleThread_HandlePacket(Packet_t * packet) {
	return; 
}
