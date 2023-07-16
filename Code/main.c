#include <stm32g071xx.h>

#include "libsys.h"
#include "libi2c.h"

#include "libpacket.h"

#define I2C_ADDR_LTR390 0xA6


int send = 0; 
unsigned int meas_uv; 
unsigned int meas_vis; 

void test(void); 
void test_send_packet(void); 

void delayms(int ms) {
	SysTick->CTRL = 0x4; 
	SysTick->LOAD = 999; 
	SysTick->VAL = 0x0; 
	SysTick->CTRL = 0x5; 
	for(int i = 0; i < ms; i++) {
		while(!(SysTick->CTRL & 0x10000)); 
//		test(); 
	}
	SysTick->CTRL = 0x4; 
}

int long_press_counter = 0; 
void shutdown(void); 

int main(void) {
	Sys_Init(); 
	
	GPIOB->BRR = 1; 
	GPIOB->BSRR = 0x1000; 
	GPIOB->OTYPER |= 0x1000; 
	GPIOB->MODER = GPIOB->MODER & 0xC0FFFFFC | 0x01000001; 
	GPIOB->PUPDR = GPIOB->PUPDR & 0xC3FFFFFF | 0x14000000; 
	
	GPIOA->BSRR = 0x00C0; 
	GPIOA->PUPDR = GPIOA->PUPDR & 0xFFFFFFF0 | 0x00000005; 
	GPIOA->MODER = GPIOA->MODER & 0xFFFF0FF0 | 0x00005000; 
	
	while(!Sys_LSEReady()); 
	GPIOB->BSRR = 1; 
	
	LPUART1_Init(); 
	I2C_HWInit(); 
	
	while(1) {
		delayms(300); 
		
		if(!(GPIOB->IDR & 0x2000)) // CHRG
			GPIOA->BRR = 0x0080; 
		delayms(40); 
		GPIOA->BSRR = 0x0080; 
		delayms(160); 
		
		if(!(GPIOB->IDR & 0x4000)) // FULL
			GPIOA->BRR = 0x0040; 
		delayms(40); 
		GPIOA->BSRR = 0x0040; 
		delayms(160); 
		
		if(!(GPIOA->IDR & 0x0001)) { // BTN MAIN
			long_press_counter++; 
			if(long_press_counter > 4) 
				shutdown(); 
		}
		else {
			long_press_counter = 0; 
		}
		
		delayms(300); 
		test_send_packet(); 
	}
}


void I2C_WriteSingleRegister(unsigned char devaddr, unsigned char regaddr, unsigned char data) {
	unsigned char buf[1]; 
	while(I2C_State.busy); 
	I2C_Data.devaddr = devaddr; 
	I2C_Data.oplen = 1; 
	I2C_Data.opcode = regaddr; 
	I2C_Data.wrlen = 1; 
	I2C_Data.wrbuf = buf; 
	I2C_Data.rdlen = 0; 
	buf[0] = data; 
	I2C_Session(); 
	while(I2C_State.busy); 
}

unsigned int I2C_ReadRegisters3(unsigned char devaddr, unsigned char regaddr) {
	unsigned char buf[3]; 
	while(I2C_State.busy); 
	I2C_Data.devaddr = devaddr; 
	I2C_Data.oplen = 1; 
	I2C_Data.opcode = regaddr; 
	I2C_Data.wrlen = 0; 
	I2C_Data.rdlen = 3; 
	I2C_Data.rdbuf = buf; 
	I2C_Session(); 
	while(I2C_State.busy); 
	unsigned int data = 0; 
	data |= buf[0]; 
	data |= buf[1] << 8; 
	data |= buf[2] << 16; 
	return data; 
}

unsigned int LTR390_Meas_UV(void) {
	I2C_WriteSingleRegister(I2C_ADDR_LTR390, 0x04, 0x26); // 18bit, 2000ms
	I2C_WriteSingleRegister(I2C_ADDR_LTR390, 0x05, 0x01); // 3x
	I2C_WriteSingleRegister(I2C_ADDR_LTR390, 0x00, 0x0A); // UV measurement
	delayms(110); 
	I2C_WriteSingleRegister(I2C_ADDR_LTR390, 0x00, 0x00); // Stop
	return I2C_ReadRegisters3(I2C_ADDR_LTR390, 0x10); 	
}

unsigned int LTR390_Meas_VIS(void) {
	I2C_WriteSingleRegister(I2C_ADDR_LTR390, 0x04, 0x26); // 18bit, 2000ms
	I2C_WriteSingleRegister(I2C_ADDR_LTR390, 0x05, 0x01); // 3x
	I2C_WriteSingleRegister(I2C_ADDR_LTR390, 0x00, 0x02); // VIS measurement
	delayms(110); 
	I2C_WriteSingleRegister(I2C_ADDR_LTR390, 0x00, 0x00); // Stop
	return I2C_ReadRegisters3(I2C_ADDR_LTR390, 0x0D); 	
}


void test(void) {
	if(send) {
		test_send_packet(); 
		send = 0; 
	}	
}

int serial_consumer(void * context, unsigned char data); 
void test_send_packet(void) {
	GPIOB->BRR = 1; 
	meas_uv = LTR390_Meas_UV(); 
	meas_vis = LTR390_Meas_VIS(); 
	GPIOB->BSRR = 1; 
	
	unsigned char buffer[6]; 
	Packet_t packet; 
	packet.dir = 1; 
	packet.pid = 0x20; 
	packet.len = 6; 
	packet.payload = buffer; 
	
	buffer[0] = meas_vis >> 16; 
	buffer[1] = meas_vis >> 8; 
	buffer[2] = meas_vis; 
	buffer[3] = meas_uv >> 16; 
	buffer[4] = meas_uv >> 8; 
	buffer[5] = meas_uv; 
	
	serial_consumer(0, '#'); 
	Packet_Encode(&packet, serial_consumer, 0); 
	serial_consumer(0, '\r'); 
	serial_consumer(0, '\n'); 
}

int serial_consumer(void * context, unsigned char data) {
	while(!(LPUART1->ISR & USART_ISR_TXE_TXFNF)); 
	LPUART1->TDR = data; 
	return 0; 
}

void shutdown(void) {
	// Disable power to sensor and BLE
	GPIOB->BRR = 0x0200; 
	GPIOF->BRR = 0x0003; 
	// Flash LED
	delayms(500); 
	for(int i = 0; i < 5; i++) {
		GPIOA->BRR = 0x0040; 
		delayms(50); 
		GPIOA->BSRR = 0x0040; 
		delayms(200); 
	}
	// Configuring IO ports
	PWR->PUCRA = 0x00C0; 
	PWR->PDCRA = 0x0000; 
	PWR->PUCRB = 0x0001; 
	PWR->PDCRB = 0x0200; 
	PWR->PUCRC = 0x0000; 
	PWR->PDCRC = 0x0000; 
	PWR->PUCRD = 0x0000; 
	PWR->PDCRD = 0x0000; 
	PWR->PUCRF = 0x0000; 
	PWR->PDCRF = 0x0003; 
	// Enter shutdown mode
	PWR->CR1 |= 0x4; 
	SCB->SCR |= 0x40; 
	__disable_irq(); 
	while(1) {
		__WFI(); 
		GPIOB->BRR = 0x1; 
		delayms(1000); 
		GPIOB->BSRR = 0x1; 
		delayms(100); 
	}
}

