#ifndef LIBSYS_H__
#define LIBSYS_H__

extern void Sys_Init(void); 

extern int  Sys_LSEReady(void); 

extern void Sys_EnterSleepMode(void); 
extern void Sys_EnterStopMode(void); 

extern void Sys_SensorOn(void); 
extern void Sys_SensorOff(void); 
extern void Sys_BluetoothOn(void); 
extern void Sys_BluetoothOff(void); 

extern int Sys_IsCharging(void); 
extern int Sys_IsChargingDone(void); 

extern void LED_Red_On(void); 
extern void LED_Red_Off(void); 
extern void LED_Green_On(void); 
extern void LED_Green_Off(void); 
extern void LED_Blue_On(void); 
extern void LED_Blue_Off(void); 

#endif
