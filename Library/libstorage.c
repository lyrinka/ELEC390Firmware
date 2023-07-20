#include "libstorage.h"

#include "libmutex.h"

Storage_t Storage; 

#define STORAGE_ARRAY_SIZE 10080 // 7 days
LWTDAQ_CompressedMeasurement_t Storage_Array[10080]; 

void Storage_Init(void) {
	Storage.array = Storage_Array; 
	Storage.capacity = STORAGE_ARRAY_SIZE; 
	Storage.startMinutes = 0; 
	Storage.currentIndex = 0; 
	for(int i = 0; i < STORAGE_ARRAY_SIZE; i++) {
		Storage_Array[i] = (LWTDAQ_CompressedMeasurement_t){
			0xFF, 0xFF, 
		}; 
	}
}

unsigned int Storage_GetPeriodStart(void) {
	unsigned int val; 
	critEnter(); 
	if(Storage.startMinutes < Storage.capacity) {
		val = Storage.startMinutes; 
		critExit(); 
		return val; 
	}
	val = Storage.startMinutes + Storage.currentIndex - Storage.capacity + 1; 
	critExit(); 
	return val; 
}

unsigned int Storage_GetPeriodEnd(void) {
	critEnter(); 
	unsigned int val = Storage.startMinutes + Storage.currentIndex; 
	critExit(); 
	return val;
}

LWTDAQ_CompressedMeasurement_t Storage_Read(unsigned int minute) {
	return Storage.array[minute % Storage.capacity];  
}

void Storage_Append(LWTDAQ_CompressedMeasurement_t data) {
	critEnter(); 
	Storage.array[Storage.currentIndex] = data; 
	if(++Storage.currentIndex > Storage.capacity) {
		Storage.startMinutes += Storage.capacity; 
		Storage.currentIndex = 0; 
	}
	critExit(); 
}
