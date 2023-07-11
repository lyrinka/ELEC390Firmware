#include <stm32g071xx.h>

#include "libsys.h"

/* System initialization:
		- Enable necessary peripherals
		- Adjust system clock for LPRUN mode
		- Necessary miscellaneous system configurations
		- Necessary system GPIO configurations
*/
void Sys_Init(void) {
	// Enable core peripherals
	RCC->AHBENR  |= RCC_AHBENR_FLASHEN; 
	RCC->APBENR1 |= RCC_APBENR1_PWREN; 
	RCC->APBENR2 |= RCC_APBENR2_SYSCFGEN; 
	RCC->IOPENR = 0x2F; // Turn on ALL GPIO
	__DSB(); 
	
	
	/* -------- Clock configurations -------- */
	
	// LSE ON
	PWR->CR1 |= PWR_CR1_DBP; 
	RCC->BDCR |= RCC_BDCR_LSEON; 
	
	// Enter LPRUN mode @1MHz
	// HSIDIV = 16, frequency 1MHz
	RCC->CR |= 0x2000; 
	__DSB(); 
	// Force LP mode
	PWR->CR1 |= 0x4000; 
	__DSB(); 
	
	/* -------- System misc configurations -------- */
	
	// Release UCPD resistors
	SYSCFG->CFGR1 |= 0x0600; 
	
	
	/* -------- GPIO configurations -------- */
	
	// Battery:
	//	PB13: Input Pull-up
	//	PB14: Input Pull-up
//GPIOB->MODER = GPIOB->MODER & 0xC3FFFFFF; 
//GPIOB->PUPDR = GPIOB->PUPDR & 0xC3FFFFFF | 0x14000000; 
	
	// Sensor VDD:
	//	PB9: Output Push-pull, Max speed
	GPIOB->OSPEEDR = GPIOB->OSPEEDR & 0xFFF3FFFF | 0x000C0000; 
	GPIOB->MODER = GPIOB->MODER & 0xFFF3FFFF | 0x00040000; 
	GPIOB->BSRR = 0x0200; 
	
	// BLE VDD:
	//	PF0: Output Push-pull, Max speed
	//	PF1: Output Push-pull, Max speed
	GPIOF->OSPEEDR = GPIOF->OSPEEDR & 0xFFFFFFF0 | 0x0000000F; 
	GPIOF->MODER = GPIOF->MODER & 0xFFFFFFF0 | 0x0000005; 
	GPIOF->BSRR = 0x0003; 
	
	// TODO: properly power up peripherals
	Sys_SensorOn(); 
	Sys_BluetoothOn(); 
}

int Sys_LSEReady(void) {
	return (RCC->BDCR & RCC_BDCR_LSERDY) ? 1 : 0; 
}

void Sys_EnterSleepMode(void) {
	__WFI(); 
}

void Sys_EnterStopMode(void) {
	// TODO: implement this
	__WFI(); 
}

void Sys_SensorOn(void) {
	GPIOB->BSRR = 0x0200; 
}

void Sys_SensorOff(void) {
	GPIOB->BRR = 0x0200; 
}

void Sys_BluetoothOn(void) {
	GPIOF->BSRR = 0x0003; 
}

void Sys_BluetoothOff(void) {
	GPIOF->BRR = 0x0003; 
}

