#ifndef LIBSTORAGE_H__
#define LIBSTORAGE_H__

#include "lwtdaq.h"

typedef struct {
	LWTDAQ_CompressedMeasurement_t * array; 
	unsigned int capacity; 
	unsigned int startMinutes; 
	unsigned int currentIndex; 
} Storage_t; 

extern Storage_t Storage; 

void Storage_Init(void); 

extern unsigned int Storage_GetPeriodStart(void); 
extern unsigned int Storage_GetPeriodEnd(void); 

extern LWTDAQ_CompressedMeasurement_t Storage_Read(unsigned int minute); 

extern void Storage_Append(LWTDAQ_CompressedMeasurement_t data); 

#endif
