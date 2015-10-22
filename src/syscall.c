#include "syscall.h"
#include "util.h"
#include "hw.h"
#include "sched.h"
#include "asm_tools.h"
 
void __attribute__((naked)) swi_handler(void)
{
	__asm("stmfd sp!, {r0-r12, lr}");
	
	//we need to save the sp pointer
	int* pointeurSp; 
	__asm("mov %0,sp" :"=r"(pointeurSp));
	
	int test;
	__asm("mov %0,r0" :"=r"(test) : : "r0", "r1");
	
		
	if (test == 1)
	{
		do_sys_reboot();
	}
	else if (test == 2)
	{
		do_sys_nop();
	}
	else if (test == 3)
	{
		do_sys_settime(pointeurSp);
	}
	else if (test == 4)
	{
		do_sys_gettime(pointeurSp);
	}
	else if (test == 5)
	{
		do_sys_yieldto(pointeurSp);
	}
	else if (test == 6)
	{
		do_sys_yield(pointeurSp);
	}
	else if (test == 7)
	{
		do_sys_exit(pointeurSp);
	}
	else
	{
		PANIC();
	}
	__asm("ldmfd sp!, {r0-r12, pc}^");
}

void irq_handler(void)
{
    set_next_tick_default();
    ENABLE_TIMER_IRQ();
    ENABLE_IRQ();
}

void sys_reboot()
{
	__asm("mov r0, #1");
	__asm("SWI #0");
}
void sys_nop()
{
	__asm("mov r0, #2");
	__asm("SWI #0");
}

void sys_settime(uint64_t date_ms)
{
	//According to call convention, date_ms data is dispatched into the two first registers r0 and r1
	__asm("mov r2, r1": : : "r0");
	__asm("mov r1, r0": : : "r2");
	
	//now we can use r0
	__asm("mov r0, #3" : : : "r1", "r2");
	
	__asm("SWI #0");
}

uint64_t sys_gettime()
{
	__asm("mov r0, #4");
	
	__asm("SWI #0");
	
	uint64_t result;
	uint32_t highbits;
	uint32_t lowbits;
	__asm("mov %0,r0" :"=r"(lowbits) : : "r1");
	__asm("mov %0,r1" :"=r"(highbits) : : "r1");
	result = highbits;
	result = result << 32;
	result += lowbits;
	return result;
	

}

void do_sys_reboot()
{
	__asm("b 0x8000");
}

void do_sys_nop()
{
	
}

void do_sys_settime(int* pointeurSp)
{
	uint64_t date_ms;
	date_ms =   pointeurSp[2];
	date_ms = date_ms << 32;
	date_ms +=  pointeurSp[1];
	set_date_ms(date_ms);
}

void do_sys_gettime(int* pointeurSp)
{
	uint32_t highbits = (get_date_ms() & 0xFFFFFFFF00000000) >> 32;
	uint32_t lowbits = (get_date_ms() & 0x00000000FFFFFFFF);
	pointeurSp[0] = lowbits;
	pointeurSp[1] = highbits;
	
}
