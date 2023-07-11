#include <stm32g071xx.h>

#include "libfifo.h"

#define critEnter() unsigned int __X_IE = __get_PRIMASK(); __disable_irq()
#define critExit() __set_PRIMASK(__X_IE)

void Fifo_Init(Fifo_t * obj, unsigned char * buffer, unsigned short capacity) {
	obj->buffer = buffer; 
	obj->capacity = capacity; 
	obj->count = 0; 
	obj->writeptr = 0; 
	obj->readptr = 0; 
}

int Fifo_Put(Fifo_t * obj, unsigned char data) {
	critEnter(); 
	int capacity = obj->capacity; 
	int count = obj->count; 
	if(count >= capacity) {
		critExit(); 
		return FIFO_FULL; 
	}
	obj->count = count + 1; 
	int pointer = obj->writeptr; 
	obj->buffer[pointer] = data; 
	if(++pointer >= capacity) 
		pointer = 0; 
	obj->writeptr = pointer; 
	critExit(); 
	return FIFO_SUCCESS; 
}

int Fifo_Peek(const Fifo_t * obj) {
	critEnter(); 
	int count = obj->count; 
	if(count <= 0) {
		critExit(); 
		return FIFO_EMPTY; 
	}
	int data = obj->buffer[obj->readptr]; 
	critExit(); 
	return data; 
}

int Fifo_Get(Fifo_t * obj) {
	critEnter(); 
	int count = obj->count; 
	if(count <= 0) {
		critExit(); 
		return FIFO_EMPTY; 
	}
	obj->count = count - 1; 
	int pointer = obj->readptr; 
	int data = obj->buffer[pointer]; 
	if(++pointer >= obj->capacity) 
		pointer = 0; 
	obj->readptr = pointer; 
	critExit(); 
	return data; 
}

