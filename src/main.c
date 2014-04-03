#include "fault_handler.h"

int main(void)
{
   int a = 10;
   int b = 0;
   int c;
   
   SysTick_Config(SystemCoreClock/100);
   
   SCB->CCR |= 0x10;
   
   c = divide_by_zero(a, b);
   
   while(1);
}
