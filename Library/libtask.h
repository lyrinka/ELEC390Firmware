#ifndef LIBTASK_H__
#define LIBTASK_H__

typedef struct {
	unsigned char * stackPointer; 
	unsigned char * stackBase; 
	unsigned int stackSize; 
} Task_t;

extern void Task_Init(void); 

extern void Task_InitializeTask(Task_t * task, unsigned char * stackBase, unsigned int stackSize, void (*entryPoint)(Task_t *)); 

extern void Task_Dispatch(Task_t * task); 
extern void Task_Yield(void); 

#endif
