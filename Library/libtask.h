#ifndef LIBTASK_H__
#define LIBTASK_H__

typedef void (*Task_Runnable_t)(void); 

typedef struct {
	unsigned char * stackPointer; 
	unsigned char * stackBase; 
	unsigned int stackSize; 
} Task_t;

extern void Task_Init(void); 

extern void Task_InitStack(Task_t * task, unsigned char * stackBase, unsigned int stackSize, Task_Runnable_t entryPoint); 

extern void Task_Dispatch(Task_t * task); 
extern void Task_Yield(void); 

#endif
