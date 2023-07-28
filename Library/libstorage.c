#include "libstorage.h"

Storage_t Storage; 

void Storage_Init(void) {
	Storage.index = 0; 
	Storage.wraps = 0; 
	// Test code filling array
	for(int i = 0; i < STORAGE_ARRAY_SIZE; i++) {
		Storage.array[i] = (DAQ_OptiMeasCM_t){
			0xFF, 0xFF, 
		}; 
	}
}

void Storage_GetRecordedRange(unsigned int * begin, unsigned int * count) {
	if(Storage.wraps == 0) {
		*begin = 0; 
		*count = Storage.index; 
	}
	else {
		*begin = (Storage.wraps - 1) * STORAGE_ARRAY_SIZE + Storage.index; 
		*count = STORAGE_ARRAY_SIZE; 
	}
}

int Storage_Read(unsigned int sample, DAQ_OptiMeasCM_t * data) {
	unsigned int begin, count, end; 
	Storage_GetRecordedRange(&begin, &count); 
	if(sample < begin) return STORAGE_READ_OUTOFBOUNDS; 
	end = begin + count; 
	if(sample >= end) return STORAGE_READ_OUTOFBOUNDS; 
	unsigned int index = sample; 
	if(Storage.wraps > 0) {
		index -= (Storage.wraps - 1) * STORAGE_ARRAY_SIZE; 
		if(index > STORAGE_ARRAY_SIZE) 
			index -= STORAGE_ARRAY_SIZE; 
	}
	if(index >= STORAGE_ARRAY_SIZE) {
		__nop(); 
	}
	*data = Storage.array[index]; 
	return STORAGE_READ_SUCCESS; 
}

unsigned int Storage_Append(const DAQ_OptiMeasCM_t * data) {
	Storage.array[Storage.index] = *data; 
	if(++Storage.index >= STORAGE_ARRAY_SIZE) {
		Storage.index = 0; 
		Storage.wraps++; 
	}
	if(Storage.wraps == 0 && Storage.index == 0) return 0; 
	return Storage.wraps * STORAGE_ARRAY_SIZE + Storage.index - 1; 
}
