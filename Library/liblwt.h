#ifndef LIBLWT_H__
#define LIBLWT_H__

// Type: LWT object
typedef struct {
	unsigned char * stackPointer; 
	unsigned char * stackBase; 
	unsigned int stackSize; 
	void * entryPoint; 
} LWT_t;

// Object: This component has profiling data: LWT_Profiling
// Object: LWT self reference (seen from LWT), last LWT (seen from kernel)
extern LWT_t * LWT_Current; 

// Procedure: LWT static initialization
extern void LWT_Init(void); 

// Procedure: LWT object construction
extern void LWT_New(LWT_t * lwt, unsigned char * stackBase, unsigned int stackSize, void * entryPoint); 

// Procedure: LWT dispatching and yielding
// Always call from kernel (main thread)
extern int LWT_Dispatch1(LWT_t * lwt); 
extern int LWT_Dispatch2(LWT_t * lwt, int param); 
// Always call from sub LWTs
extern int LWT_Yield1(void); 
extern int LWT_Yield2(int param); 

#endif
