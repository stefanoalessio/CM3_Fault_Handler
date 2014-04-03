/**
 * \file
 * \brief Hard Fault Management
 *
 * This module gives information about a hard fault exception.
 * There are also some functions to generate exceptions, so you can call them
 * and have an idea of what help this module can give you!
 */
#include <stdio.h>
#include "fault_handler.h"

/*
 * Private defines
 */

#define     __IO    volatile      /*!< defines 'read / write' permissions   */
#define     __I    volatile       /*!< defines 'read only' permissions   */
/** 
  memory mapped structure for System Control Block (SCB)
  @{
 */
typedef struct {
	__I  uint32_t CPUID;     /**< Offset: 0x00  CPU ID Base Register                                  */
	__IO uint32_t ICSR;      /**< Offset: 0x04  Interrupt Control State Register                      */
	__IO uint32_t VTOR;      /**< Offset: 0x08  Vector Table Offset Register                          */
	__IO uint32_t AIRCR;     /**< Offset: 0x0C  Application Interrupt / Reset Control Register        */
	__IO uint32_t SCR;       /**< Offset: 0x10  System Control Register                               */
	__IO uint32_t CCR;       /**< Offset: 0x14  Configuration Control Register                        */
	__IO uint8_t  SHP[12];   /**< Offset: 0x18  System Handlers Priority Registers (4-7, 8-11, 12-15) */
	__IO uint32_t SHCSR;     /**< Offset: 0x24  System Handler Control and State Register             */
	__IO uint32_t CFSR;      /**< Offset: 0x28  Configurable Fault Status Register                    */
	__IO uint32_t HFSR;      /**< Offset: 0x2C  Hard Fault Status Register                            */
	__IO uint32_t DFSR;      /**< Offset: 0x30  Debug Fault Status Register                           */
	__IO uint32_t MMFAR;     /**< Offset: 0x34  Mem Manage Address Register                           */
	__IO uint32_t BFAR;      /**< Offset: 0x38  Bus Fault Address Register                            */
	__IO uint32_t AFSR;      /**< Offset: 0x3C  Auxiliary Fault Status Register                       */
	__I  uint32_t PFR[2];    /**< Offset: 0x40  Processor Feature Register                            */
	__I  uint32_t DFR;       /**< Offset: 0x48  Debug Feature Register                                */
	__I  uint32_t ADR;       /**< Offset: 0x4C  Auxiliary Feature Register                            */
	__I  uint32_t MMFR[4];   /**< Offset: 0x50  Memory Model Feature Register                         */
	__I  uint32_t ISAR[5];   /**< Offset: 0x60  ISA Feature Register                                  */
} SCB_Type;

#define SCS_BASE            (0xE000E000)            /**< System Control Space Base Address */
#define SCB_BASE            (SCS_BASE +  0x0D00)    /**< System Control Block Base Address */
#define SCB                 ((SCB_Type *) SCB_BASE) /**< SCB configuration struct          */

/* Bit definition for SCB_CFSR register */
/**< MFSR */
#define  SCB_CFSR_IACCVIOL     ((uint32_t)0x00000001) /**< Instruction access violation */
#define  SCB_CFSR_DACCVIOL     ((uint32_t)0x00000002) /**< Data access violation */
#define  SCB_CFSR_MUNSTKERR    ((uint32_t)0x00000008) /**< Unstacking error */
#define  SCB_CFSR_MSTKERR      ((uint32_t)0x00000010) /**< Stacking error */
#define  SCB_CFSR_MMARVALID    ((uint32_t)0x00000080) /**< Memory Manage Address Register address valid flag */
/**< BFSR */
#define  SCB_CFSR_IBUSERR      ((uint32_t)0x00000100) /**< Instruction bus error flag */
#define  SCB_CFSR_PRECISERR    ((uint32_t)0x00000200) /**< Precise data bus error */
#define  SCB_CFSR_IMPRECISERR  ((uint32_t)0x00000400) /**< Imprecise data bus error */
#define  SCB_CFSR_UNSTKERR     ((uint32_t)0x00000800) /**< Unstacking error */
#define  SCB_CFSR_STKERR       ((uint32_t)0x00001000) /**< Stacking error */
#define  SCB_CFSR_BFARVALID    ((uint32_t)0x00008000) /**< Bus Fault Address Register address valid flag */
/**< UFSR */
#define  SCB_CFSR_UNDEFINSTR   ((uint32_t)0x00010000) /**< The processor attempt to excecute an undefined instruction */
#define  SCB_CFSR_INVSTATE     ((uint32_t)0x00020000) /**< Invalid combination of EPSR and instruction */
#define  SCB_CFSR_INVPC        ((uint32_t)0x00040000) /**< Attempt to load EXC_RETURN into pc illegally */
#define  SCB_CFSR_NOCP         ((uint32_t)0x00080000) /**< Attempt to use a coprocessor instruction */
#define  SCB_CFSR_UNALIGNED    ((uint32_t)0x01000000) /**< Fault occurs when there is an attempt to make an unaligned memory access */
#define  SCB_CFSR_DIVBYZERO    ((uint32_t)0x02000000) /**< Fault occurs when SDIV or DIV instruction is used with a divisor of 0 */


/*
 * Private Functions
 */
static void printErrorMsg(const char * errMsg);
static void printUsageErrorMsg(uint32_t CFSRValue);
static void printBusFaultErrorMsg(uint32_t CFSRValue);
static void printMemoryManagementErrorMsg(uint32_t CFSRValue);
static void DumpStack(uint32_t stack[]);
static void HardFaultHandlerUser(uint32_t stack[]);


/**
 * \brief The Hard Fault Handler
 *
 */
void Hard_Fault_Handler(uint32_t stack[])
{
	static char msg[80];
	printErrorMsg("Hard Fault!!!\n");
	sprintf(msg, "SCB->HFSR = 0x%08x\n", SCB->HFSR);
	printErrorMsg(msg);

	if ((SCB->HFSR & (1 << 30)) != 0) {
		printErrorMsg("Forced Hard Fault\n");
		sprintf(msg, "SCB->CFSR = 0x%08x\n", SCB->CFSR);
		printErrorMsg(msg);

		if ((SCB->CFSR & 0xFFFF0000) != 0) {
			printUsageErrorMsg(SCB->CFSR);
		}

		if ((SCB->CFSR & 0xFF00) != 0) {
			printBusFaultErrorMsg(SCB->CFSR);
		}

		if ((SCB->CFSR & 0xFF) != 0) {
			printMemoryManagementErrorMsg(SCB->CFSR);
		}
	}

	DumpStack(stack);
	HardFaultHandlerUser(stack);

#if defined(__ICCARM__)
	__asm volatile("BKPT #01");
#endif

	while (1) {};
}

/**
 * \brief Fill-in this function with your code to handle the exception
 *
 * It is called after debug string prints
 */
static void HardFaultHandlerUser(uint32_t stack[])
{
	/* Application specific code */
}

/**
 * \brief Print Messages using semihosting
 */
static void printErrorMsg(const char * errMsg)
{
	printf("%s", errMsg);
}

/**
 * \brief Print Usage Errors
 */
static void printUsageErrorMsg(uint32_t CFSRValue)
{
	printErrorMsg("Usage fault: ");

	if ((CFSRValue & SCB_CFSR_DIVBYZERO) != 0) {
		printErrorMsg("Divide by zero\n");
	}

	if ((CFSRValue & SCB_CFSR_INVSTATE) != 0) {
		printErrorMsg("Invalid combination of EPSR and instruction,\nsuch as calling a null pointer function\n");
	}

	if ((CFSRValue & SCB_CFSR_UNDEFINSTR) != 0) {
		printErrorMsg("The processor attempted to excecute an undefined instruction\n");
	}

	if ((CFSRValue & SCB_CFSR_INVPC) != 0) {
		printErrorMsg("Attempt to load EXC_RETURN into pc illegally\n");
	}

	if ((CFSRValue & SCB_CFSR_NOCP) != 0) {
		printErrorMsg("Attempt to use a coprocessor instruction\n");
	}

	if ((CFSRValue & SCB_CFSR_UNALIGNED) != 0) { /* if enabled it is for all instruction, otherwise only for some of them */
		printErrorMsg("Attempt to make an unaligned memory access\n");
	}
}

/**
 * \brief Print Bus Fault Errors
 */
static void printBusFaultErrorMsg(uint32_t CFSRValue)
{
	char str[200];
	printErrorMsg("Bus fault: ");
	CFSRValue = (CFSRValue & 0x0000FF00);
	sprintf(str, "%.2X\n", CFSRValue);
	printErrorMsg(str);

	if ((CFSRValue & SCB_CFSR_IBUSERR) != 0) {
		printErrorMsg("Instruction bus error\n");
	}

	if ((CFSRValue & SCB_CFSR_PRECISERR) != 0) {
		printErrorMsg("Precise data bus error\n");
	}

	if ((CFSRValue & SCB_CFSR_IMPRECISERR) != 0) {
		printErrorMsg("Imprecise data bus error\n");
	}

	if ((CFSRValue & SCB_CFSR_UNSTKERR) != 0) {
		printErrorMsg("Unstacking error\n");
	}

	if ((CFSRValue & SCB_CFSR_STKERR) != 0) {
		printErrorMsg("Stacking error\n");
	}

	if ((CFSRValue & SCB_CFSR_BFARVALID) != 0) {
		sprintf(str, "Bus Fault Address Register address valid flag\nBFAR value = 0x%.8X\n", SCB->BFAR);
		printErrorMsg(str);
	}
}

/**
 * \brief Print Memory Management Errors
 */
static void printMemoryManagementErrorMsg(uint32_t CFSRValue)
{
	char str[200];
	printErrorMsg("Memory Management (MPU) fault: ");
	CFSRValue &= 0x000000FF; /* mask mem faults only */
	sprintf(str, "%.2X\n", CFSRValue);
	printErrorMsg(str);

	if ((CFSRValue & SCB_CFSR_IACCVIOL) != 0) {
		printErrorMsg("Instruction access violation\n");
	}

	if ((CFSRValue & SCB_CFSR_DACCVIOL) != 0) {
		printErrorMsg("Data access violation\n");
	}

	if ((CFSRValue & SCB_CFSR_MUNSTKERR) != 0) {
		printErrorMsg("Unstacking error\n");
	}

	if ((CFSRValue & SCB_CFSR_MSTKERR) != 0) {
		printErrorMsg("Stacking error\n");
	}

	if ((CFSRValue & SCB_CFSR_MMARVALID) != 0) {
		sprintf(str,"Memory Manage Address Register address valid flag\nMMFAR value = 0x%.8X\n", SCB->MMFAR);
		printErrorMsg(str);
	}
}

#if defined(__CC_ARM)
__asm void HardFault_Handler(void)
{
	TST lr, #4
	ITE EQ
	MRSEQ r0, MSP
	MRSNE r0, PSP
	B __cpp(Hard_Fault_Handler)
}
#elif defined(__ICCARM__)
void HardFault_Handler(void)
{
	__asm("TST lr, #4");
	__asm("ITE EQ");
	__asm("MRSEQ r0, MSP");
	__asm("MRSNE r0, PSP");
	__asm("B Hard_Fault_Handler");
}
#else
#warning Not supported compiler type
#endif

enum { r0, r1, r2, r3, r12, lr, pc, psr};

/**
 * \brief Dump Stack, printing all registers ARM core pushes on stack on hard fault exception
 */
static void DumpStack(uint32_t stack[])
{
	static char msg[80];
	int code_address_error=0;
	sprintf(msg, "\nr0  = 0x%08x\n", stack[r0]);
	printErrorMsg(msg);
	sprintf(msg, "r1  = 0x%08x\n", stack[r1]);
	printErrorMsg(msg);
	sprintf(msg, "r2  = 0x%08x\n", stack[r2]);
	printErrorMsg(msg);
	sprintf(msg, "r3  = 0x%08x\n", stack[r3]);
	printErrorMsg(msg);
	sprintf(msg, "r12 = 0x%08x\n", stack[r12]);
	printErrorMsg(msg);
	sprintf(msg, "lr  = 0x%08x\n", stack[lr]);
	printErrorMsg(msg);
	sprintf(msg, "pc  = 0x%08x\n", stack[pc]);
	printErrorMsg(msg);
	sprintf(msg, "psr = 0x%08x\n", stack[psr]);
	printErrorMsg(msg);

	if (stack[pc] == 0) {
		code_address_error = stack[lr];
	} else {
		code_address_error = stack[pc];
	}

	sprintf(msg, "\n--\t--\t--\nHard fault occurred at address 0x%08x.\nFind high-level ", code_address_error);
	printErrorMsg(msg);
	sprintf(msg, "function with\nDisassembly window or Map file\n--\t--\t--\n");
	printErrorMsg(msg);
}


/*
 * Test routines
 */


/**
 * \brief This function does a buffer overflow
 */
uint8_t bus_fault_code(void)
{
	uint8_t array[5]= {0,0,0,0,0};
	int i;

	for (i = 1; i < 10000; i++)
		array[i] = i*array[i-1];

	return array[i];
}

/**
 * \brief This function tries to divide by zero (and enables div_by_zero trap)
 */
volatile int dontoptimize = 1;
#if defined(__ICCARM__)
#pragma optimize = none
#endif
uint8_t divide_by_zero(void)
{
	int i=4, j=0, z;
	/* Enable divide by zero trap */
	SCB->CCR |= 0x10;

	if (dontoptimize)
		z = i/j;
	else
		z = 4;

	return z;
}

/**
 * \brief This function creates a null pointer and then calls it
 */
uint8_t call_to_null_function(void)
{
	void (*func_pointer)(void);
	func_pointer = 0;
	func_pointer();
	return 0;
}

/**
 * \brief This function accesses an invalid RAM address
 */
uint8_t dangling_pointer(void)
{
	uint8_t* addr = 0x00; /* invalid pointer */
	addr[100] = 100;
	return addr[100];
}

/**
 * \brief This function accesses an RAM address usually not available
 */
uint32_t dangling_pointer2(void)
{
	unsigned long * addr = (unsigned long*)0x20200000UL; /* invalid pointer */
	addr[100] = 0xa56765aeUL;
	addr[101] = 0xa56765afUL;
	addr[102] = addr[101]-addr[100];
	return addr[102];
}