#include <stm32g071xx.h>

#include "libtask.h"

typedef struct {
	union {
		unsigned int contextFrame[8]; 
		struct {
			unsigned int r4; 
			unsigned int r5; 
			unsigned int r6; 
			unsigned int r7; 
			unsigned int r8; 
			unsigned int r9; 
			unsigned int r10; 
			unsigned int r11; 
		}; 
	}; 
	union {
		unsigned int hardwareFrame[8]; 
		struct {
			unsigned int r0; 
			unsigned int r1; 
			unsigned int r2; 
			unsigned int r3; 
			unsigned int r12; 
			unsigned int lr; 
			unsigned int pc; 
			unsigned int psr; 
		}; 
	}; 
} StackFrame_t; 

void Task_Init(void) {
	// TODO: confirm these priorities
	SCB->SHP[0] = SCB->SHP[0] & 0x0000FFFF; 
	SCB->SHP[1] = SCB->SHP[1] & 0xFFFF0000 | 0x0000C000; 
}

void Task_InitializeTask(Task_t * task, unsigned char * stackBase, unsigned int stackSize, void (*entryPoint)(Task_t *)) {
	task->stackBase = stackBase; 
	
	unsigned int stackEnd = (unsigned int)(stackBase + stackSize); 
	if(stackEnd & 0x7) stackSize -= stackEnd & 0x7;
	task->stackSize = stackSize; 
	
	
	StackFrame_t * stackFrame = (StackFrame_t *)(stackBase + stackSize - sizeof(StackFrame_t)); 
	task->stackPointer = (unsigned char *)stackFrame; 
	for(int i = 0; i < 8; i++) stackFrame->contextFrame[i] = 0xDEADBEF0 + i; 
	for(int i = 1; i < 5; i++) stackFrame->hardwareFrame[i] = 0xDEADBEE0 + i; 
	stackFrame->r0 = (unsigned int)task; 
	stackFrame->lr = 0x22222222; 
	stackFrame->pc = ((unsigned int)entryPoint) | 0x1; 
	stackFrame->psr = 0x01000000; 
}

__svc(0x0) unsigned char * internalSvcDispatch(unsigned char *); 
void Task_Dispatch(Task_t * task) {
		task->stackPointer = internalSvcDispatch(task->stackPointer); 
}

__svc(0x1) void internalSvcYield(void); 
void Task_Yield(void) {
		internalSvcYield(); 
}

__asm void SVC_Handler(void) {
		LDR			R2, =0xE000ED04 // SCB->ICSR
		LDR			R3, =0x10000000	// PENDSV_EN
		STR			R3, [R2]
		BX			LR
}

__asm void PendSV_Handler(void) {
		MOV			R0, LR
		MOVS		R1, #4
		EORS		R0, R1
		MOV			LR, R0
		TST			R0, R1 	// 0 -> return to MSP, 1 -> return to PSP
		BEQ			PendSV_Handler_PSPToMSP
PendSV_Handler_MSPToPSP
		LDR			R0, [SP]
		PUSH		{R4-R7}
		MOV			R4, R8
		MOV			R5, R9
		MOV			R6, R10
		MOV			R7, R11
		PUSH		{R4-R7}
//	LDMIA		R0!, {R4-R11}
// 	LDMIA workaround start
		LDR			R4, [R0, #16]
		LDR			R5, [R0, #20]
		LDR			R6, [R0, #24]
		LDR			R7, [R0, #28]
		MOV			R8, R4
		MOV			R9, R5
		MOV			R10, R6
		MOV			R11, R7
		LDR			R4, [R0, #0 ]
		LDR			R5, [R0, #4 ]
		LDR			R6, [R0, #8 ]
		LDR			R7, [R0, #12]
		ADDS		R0, #32
//	Workaround end
		MSR			PSP, R0
		BX			LR
PendSV_Handler_PSPToMSP
		MRS			R0, PSP
//	STMDB		R0!,{R4-R11}
//	STMDB workaround start
		SUBS		R0, #32
		STR			R4, [R0, #0 ]
		STR			R5, [R0, #4 ]
		STR			R6, [R0, #8 ]
		STR			R7, [R0, #12]
		MOV			R4, R8
		MOV			R5, R9
		MOV			R6, R10
		MOV			R7, R11
		STR			R4, [R0, #16]
		STR			R5, [R0, #20]
		STR			R6, [R0, #24]
		STR			R7, [R0, #28]
//	Workaround end
		POP			{R4-R7}
		MOV			R8, R4
		MOV			R9, R5
		MOV			R10, R6
		MOV			R11, R7
		POP			{R4-R7}
		STR			R0, [SP]
		BX			LR
}



