#include "blethread.h"

#include "libcodec.h"

#include "libsys.h"
#include "libuart.h"

BleThread_State_t BleThread_State; 

/* -------- LWT instantiation -------- */
#define BLETHREAD_STACK_SIZE 512
unsigned char BleThread_Stack[BLETHREAD_STACK_SIZE];  

void BleThread_Init(void) {
	// UART middleware
	void BleThread_SerialInit(void); 
	BleThread_SerialInit(); 
	
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
			return i + 1; 
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


/* -------- Testing code -------- */
// Line buffer is used during line parsing, message rx/tx, rx packet payload & encodings, and tx packet encodings. 
// Line buffer shall not be used to store tx packet payload.
#define BLETHREAD_LINEBUFFER_SIZE 350
unsigned char BleThread_LineBuffer[BLETHREAD_LINEBUFFER_SIZE]; 

void BleThread_Entry(void) {
	void BleThread_HandleMessage(const char * string, unsigned int length); 
	void BleThread_HandlePacket(const Packet_t * packet); 
	
	Packet_t packet; 
	packet.capacity = BLETHREAD_LINEBUFFER_SIZE > 255 ? 255 : BLETHREAD_LINEBUFFER_SIZE; 
	packet.payload = BleThread_LineBuffer; 
	for(;;) {
		int length = BleThread_ReadLine(BleThread_LineBuffer, BLETHREAD_LINEBUFFER_SIZE); 
		int status = BleThread_ParsePacket(BleThread_LineBuffer, length, &packet); 
		if(status) {
			BleThread_HandleMessage((const char *)BleThread_LineBuffer, length); 
		}
		else {
			BleThread_HandlePacket(&packet); 
		}
	}
}

void BleThread_HandleMessage(const char * string, unsigned int length) {
	
	__nop(); 
}

void BleThread_HandlePacket(const Packet_t * packet) {
	
	__nop(); 
}







