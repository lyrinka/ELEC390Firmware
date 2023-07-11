#ifndef LIBFIFO_H__
#define LIBFIFO_H__

typedef struct {
	unsigned char * buffer; 
	unsigned short capacity; 
	unsigned short count; 
	unsigned short writeptr; 
	unsigned short readptr; 
} Fifo_t; 

#define FIFO_SUCCESS 0
#define FIFO_FULL 1
#define FIFO_EMPTY -1; 

extern void Fifo_Init(Fifo_t * obj, unsigned char * buffer, unsigned short capacity); 
extern int Fifo_Put(Fifo_t * obj, unsigned char data); 
extern int Fifo_Peek(const Fifo_t * obj); 
extern int Fifo_Get(Fifo_t * obj); 

#endif
