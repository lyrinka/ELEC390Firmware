#ifndef LIBI2C_H__
#define LIBI2C_H__

#define I2C_ERR_NOERR 0
#define I2C_ERR_NACK 1
#define I2C_ERR_ARLO 2

typedef struct {
	unsigned char devaddr; 
	unsigned char oplen;
	unsigned char wrlen; 
	unsigned char rdlen; 
	unsigned int opcode; 
	const unsigned char * wrbuf; 
	unsigned char * rdbuf; 
} I2C_Data_TypeDef; 

typedef struct {
	unsigned char busy; 
	unsigned char index; 
	unsigned char error; 
} I2C_State_TypeDef; 

extern volatile I2C_Data_TypeDef I2C_Data; 
extern volatile I2C_State_TypeDef I2C_State; 

extern void I2C_HWInit(void); 
extern int I2C_Session(void); 

__weak void I2C_SessionDoneCallback(int error); 

#endif
