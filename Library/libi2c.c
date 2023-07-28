// The library is adapted from STM32G030
// TODO: there's an issue where pure zero-length requests fail
#include <stm32g071xx.h>

#include "libi2c.h"

volatile I2C_Data_TypeDef I2C_Data; 
volatile I2C_State_TypeDef I2C_State;

unsigned int profile_irq_i2c1_cnt = 0; 

void I2C_HWInit(void) {
	I2C_State.busy = 0; 
	// PB7: SDA (AF6)
	// PB8: SCL (AF6)
	RCC->IOPENR |= RCC_IOPENR_GPIOBEN; 
	GPIOB->BSRR = 0x0180; 
	GPIOB->AFR[1] = GPIOB->AFR[1] & 0xFFFFFFF0 | 0x00000006; 
	GPIOB->AFR[0] = GPIOB->AFR[0] & 0x0FFFFFFF | 0x60000000; 
	GPIOB->OTYPER |= 0x0180; 
	GPIOB->MODER = GPIOB->MODER & 0xFFFC3FFF | 0x00028000; 
	
	// I2C1@1MHz, 100kHz
	RCC->CCIPR = RCC->CCIPR & 0xFFFFCFFF; 
	RCC->APBENR1 |= RCC_APBENR1_I2C1EN; 
	__DSB(); 
	I2C1->CR1 = 0x0; 
	// Sample data from STM32126 @1MHz
	I2C1->TIMINGR = 0x00000103; 
//I2C1->TIMINGR = 0x30420F13; // 100kHz @16MHz
//I2C1->TIMINGR = 0x10320309; // 400kHz @16MHz
	// Enabled IRQ: TX/RXIE, NACKIE, TCIE, ERRIE, STOPF. DNF OFF, ANF ON. 
	I2C1->CR1 = 0x000000F7; 
	I2C1->ICR = 0x3F38; 
	
	// I2C IRQ Enable
	NVIC_ClearPendingIRQ(I2C1_IRQn); 
	NVIC_SetPriority(I2C1_IRQn, 3); 
	NVIC_EnableIRQ(I2C1_IRQn); 
}

#define ctrl_wr_noend(addr, N) 		((addr) & 0xFE) | 0x00002000 | (((N) & 0xFF) << 16)
#define ctrl_wr_autoend(addr, N) 	((addr) & 0xFE) | 0x02002000 | (((N) & 0xFF) << 16)
#define ctrl_rd_autoend(addr, N) 	((addr) & 0xFE) | 0x02002400 | (((N) & 0xFF) << 16)

int I2C_Session(void) {
	if(I2C_State.busy) return 0; 
	I2C_State.busy = 1; 
	I2C_State.index = 0; 
	I2C_State.error = 0; 
	unsigned char devaddr = I2C_Data.devaddr; 
	unsigned char effwrlen = I2C_Data.oplen + I2C_Data.wrlen; 
	unsigned char effrdlen = I2C_Data.rdlen; 
	if(effwrlen != 0) {
		if(I2C_Data.rdlen == 0) 
			I2C1->CR2 = ctrl_wr_autoend(devaddr, effwrlen); 
		else 
			I2C1->CR2 = ctrl_wr_noend(devaddr, effwrlen); 
	}
	else {
		I2C1->CR2 = ctrl_rd_autoend(devaddr, effrdlen); 
	}
	return 1; 
}

void I2C1_IRQHandler(void) {
	profile_irq_i2c1_cnt++; 
	// Clear OVR/UDR/PEC/TIMEOUT/ALERT error (irrelevant error)
	// Clear BERR (errata)
	// Only error left is ARLO
	I2C1->ICR = 0x3D00; 
	unsigned int flags = I2C1->ISR; 
	if(flags & 0x0200) { // ARLO
		I2C1->ICR = 0x0200; 
		I2C_State.error = I2C_ERR_ARLO; 
	}
	if(flags & 0x0010) { // NACK
		I2C1->ICR = 0x0010; 
		I2C_State.error = I2C_ERR_NACK; 
	}
	if(flags & 0x0020) { // STOPF
		I2C1->ICR = 0x0020; 
		if(I2C_State.busy) {
			I2C_State.busy = 0; 
			I2C_SessionDoneCallback(I2C_State.error); 
		}
	}
	if(flags & 0x0040) { // TC (TCR not used)
		// Only WR->RD stage comes here
		I2C_State.index = 0; 
		unsigned char devaddr = I2C_Data.devaddr; 
		unsigned char effrdlen = I2C_Data.rdlen; 
		I2C1->CR2 = ctrl_rd_autoend(devaddr, effrdlen); 
	}
	if(flags & 0x0002) { // TXIS
		int index = I2C_State.index++; 
		int oplen = I2C_Data.oplen; 
		if(index < oplen) {
			I2C1->TXDR = 0xFF & (I2C_Data.opcode >> ((oplen - index - 1) << 3)); 
		}
		else {
			I2C1->TXDR = I2C_Data.wrbuf[index - oplen]; 
		}
	}
	if(flags & 0x0004) { // RXNE
		int index = I2C_State.index++; 
		I2C_Data.rdbuf[index] = I2C1->RXDR; 
	}
}

__weak void I2C_SessionDoneCallback(int error) {
	return; 
}
