#ifndef LIBSTORAGE_H__
#define LIBSTORAGE_H__

#include "mainthread.h"

// 1.67 days when interval is 10 seconds (demo)
// 10 days when interval is 60 seconds
#define STORAGE_ARRAY_SIZE 14400 

typedef struct {
	unsigned int index; 
	unsigned int wraps; 
	DAQ_OptiMeasCM_t array[STORAGE_ARRAY_SIZE]; 
} Storage_t; 

extern Storage_t Storage; 

extern void Storage_Init(void); 

extern void Storage_GetRecordedRange(unsigned int * begin, unsigned int * count); 

#define STORAGE_READ_SUCCESS 0
#define STORAGE_READ_OUTOFBOUNDS -1
extern int Storage_Read(unsigned int sample, DAQ_OptiMeasCM_t * data); 

extern unsigned int Storage_Append(const DAQ_OptiMeasCM_t * data); 

#endif
