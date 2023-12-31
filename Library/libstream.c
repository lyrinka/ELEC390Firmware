#include "libstream.h"

#include "critregion.h"

void Stream_Init(Stream_t * stream, unsigned char * storage, unsigned int size) {
	stream->buffer = storage; 
	stream->capacity = size; 
	stream->head = 0; 
	stream->tail = 0; 
	stream->size = 0; 
	stream->maxSizeReached = 0; 
	stream->totalWritten = 0; 
	stream->totalRead = 0; 
	stream->totalDropped = 0; 
}

unsigned int Stream_GetSize(const Stream_t * stream) {
	return stream->size; 
}

unsigned int Stream_GetSizeRemaining(const Stream_t * stream) {
	return stream->capacity - stream->size; 
}

int Stream_Write(Stream_t * stream, unsigned char data) {
	critEnter(); 
	if(stream->size >= stream->capacity) {
		// TODO: how do we log this overflow?
		stream->totalDropped++; 
		critExit(); 
		return STREAM_WRITE_FULL; 
	}
	unsigned int head = stream->head; 
	stream->buffer[head] = data; 
	if(++head >= stream->capacity) head = 0; 
	stream->head = head; 
	if(++stream->size > stream->maxSizeReached)
		stream->maxSizeReached = stream->size; 
	stream->totalWritten++; 
	critExit(); 
	return STREAM_WRITE_SUCCESS; 
}

int Stream_Read(Stream_t * stream, unsigned char * data0) {
	critEnter(); 
	if(stream->size == 0) {
		critExit(); 
		return STREAM_READ_EMPTY; 
	}
	unsigned int tail = stream->tail; 
	unsigned char data = stream->buffer[tail]; 
	if(++tail >= stream->capacity) tail = 0; 
	stream->tail = tail; 
	stream->size--; 
	stream->totalRead++; 
	critExit(); 
	*data0 = data; 
	return STREAM_READ_SUCCESS; 
}
