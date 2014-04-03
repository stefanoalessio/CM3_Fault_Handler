#ifndef __FAULT_HANDLER_H_
#define __FAULT_HANDLER_H_

u8 bus_fault_code(void);
u8 divide_by_zero(void);
u8 call_to_null_function(void);
u8 dangling_pointer(void);
u32 dangling_pointer2(void);
#endif