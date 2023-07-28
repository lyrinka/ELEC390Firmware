#ifndef LIBLPTIM_H__
#define LIBLPTIM_H__

extern void LPTIM_Init(void); 

__weak void LPTIM_Callback(void); // IRQ context!

#endif
