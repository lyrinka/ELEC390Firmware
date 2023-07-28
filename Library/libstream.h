#ifndef LIBSTREAM_H__
#define LIBSTREAM_H__

// Type: Stream object
typedef struct {
	unsigned char * buffer; 
	unsigned int capacity; 
	unsigned int head; 
	unsigned int tail; 
	unsigned int size; 
	unsigned int maxSizeReached; 
	unsigned int totalWritten; 
	unsigned int totalRead; 
	unsigned int totalDropped; 
} Stream_t; 

// Procedure: Stream object construction
extern void Stream_Init(Stream_t * stream, unsigned char * storage, unsigned int size); 

// Procedure: Stream property getters
extern unsigned int Stream_GetSize(const Stream_t * stream); 
extern unsigned int Stream_GetSizeRemaining(const Stream_t * stream); 

// Procedure: Stream writes
#define STREAM_WRITE_SUCCESS 0
#define STREAM_WRITE_FULL -1
extern int Stream_Write(Stream_t * stream, unsigned char data); 

// Procedure: Stream reads
#define STREAM_READ_SUCCESS 0
#define STREAM_READ_EMPTY -1
extern int Stream_Read(Stream_t * stream, unsigned char * data); 

#endif
