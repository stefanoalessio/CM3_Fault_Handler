#ifndef __FAULT_HANDLER_H_
#define __FAULT_HANDLER_H_

#include <stdint.h>

uint8_t bus_fault_code(void);
uint8_t divide_by_zero(void);
uint8_t call_to_null_function(void);
uint8_t dangling_pointer(void);
uint32_t dangling_pointer2(void);

#endif