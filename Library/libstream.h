#ifndef LIBSTREAM_H__
#define LIBSTREAM_H__

typedef struct {
	unsigned int written; 
	unsigned int read; 
	unsigned int dropped; 
} Stream_Profiling_t; 

typedef struct {
	Stream_Profiling_t * profiling; 
	unsigned char * buffer; 
	unsigned int capacity; 
	unsigned int head; 
	unsigned int tail; 
	unsigned int size; 
	unsigned int maxSizeReached; 
} Stream_t; 

extern void Stream_Init(Stream_t * stream, unsigned char * storage, unsigned int size); 
extern void Stream_AttachProfileObj(Stream_t * stream, Stream_Profiling_t * profiling); 

#define STREAM_WRITE_SUCCESS 0
#define STREAM_WRITE_FULL -1
extern int Stream_Write(Stream_t * stream, unsigned char data); 

#define STREAM_READ_EMPTY -1
extern int Stream_Read(Stream_t * stream); 

#endif
