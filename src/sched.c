#include "sched.h"
#include "kheap.h"
#include "hw.h"
#include "asm_tools.h"

struct pcb_s *current_process;
struct pcb_s kmain_process;

void sched_init()
{
	current_process = &kmain_process;
	// we link kmain_process on itself, because it's a circular linked list
	kmain_process.nextPcb = &kmain_process;
	kmain_process.previousPcb = &kmain_process;
	kheap_init();
	
}

void sys_yieldto(struct pcb_s* dest)
{
	__asm("mov r1, r0");
	__asm("mov r0, #5");
	__asm("SWI #0");
}

void sys_yield()
{
	__asm("mov r0, #6");
	__asm("SWI #0");
}

void sys_exit(int status)
{
	__asm("mov r1, r0");
	__asm("mov r0, #7");
	__asm("SWI #0");
}


void do_sys_yieldto(int* pointeurSp)
{
	int i =0;
	
	//get the called process pcb adress
	struct pcb_s * dest;
	dest = (struct pcb_s *)pointeurSp[1];
	
	for(i=0;i<13;i++)
	{
		//save caller process context
		current_process->registres[i] = (uint32_t)pointeurSp[i];
		
		// load called process context
		pointeurSp[i] = (uint32_t) dest->registres[i];
	}
	current_process->lrSvc=(uint32_t)pointeurSp[13]; // we save the lrSvc from the stack, might use it later
	
	
	__asm("mrs r7, SPSR"); //we save the CPSR (which is in the SPSR)
	__asm("mov %0, r7": "=r"(current_process->CPSR) : :);
	__asm("mov r7, %0": : "r"(dest->CPSR) :);
	__asm("msr SPSR, r7"); //we load the CPSR
	
	__asm("mov r5, %0" : : "r"(dest->sp) :); // we store dest because we will not be able to access it when changing to sys mod
	__asm("mov r6, %0" : : "r"(dest->lrUser) : "r5");
	
	__asm("cps 0b11111");//switch to system mode
	__asm("mov %0, sp" : "=r"(current_process->sp) : : "r5", "r6"); //we save the sp from Sys Mode
	__asm("mov %0, lr" : "=r"(current_process->lrUser) : : "sp", "r5", "r6"); //we save the LR from Sys Mode	
	
	__asm("mov sp, r5"); //we load the sp 
	__asm("mov lr, r6"); // we load the lr
	__asm("cps 0b10011");//switch back to svc mode
	
	
	current_process = dest;
	
	//here everything have been made using the stack. The swi handler will then 
	//reload the modified stack unto the registers
}

void do_sys_yield(int* pointeurSp)
{
	int i =0;
	
	//--------------save----------------------
	
	for(i=0;i<13;i++)
	{
		//save caller process context
		current_process->registres[i] = (uint32_t)pointeurSp[i];
	}
	current_process->lrSvc=(uint32_t)pointeurSp[13]; // we save the lrSvc from the stack, might use it later
	
	__asm("mrs r7, SPSR"); //we save the CPSR (which is in the SPSR)
	__asm("mov %0, r7": "=r"(current_process->CPSR) : :);
	
	__asm("cps 0b11111");//switch to system mode
	__asm("mov %0, sp" : "=r"(current_process->sp) : :); //we save the sp from Sys Mode
	__asm("mov %0, lr" : "=r"(current_process->lrUser) : :); //we save the LR from Sys Mode	
	__asm("cps 0b10011");//switch back to svc mode
	//---------------------------------------
	
	elect();
	
	//--------------load----------------------
	for(i=0;i<13;i++)
	{
		// load called process context
		pointeurSp[i] = (uint32_t) current_process->registres[i];
	}
	
	__asm("mov r7, %0": : "r"(current_process->CPSR) :);
	__asm("msr SPSR, r7"); //we load the CPSR
	
	__asm("cps 0b11111");//switch to system mode
	__asm("mov sp, %0": : "r"(current_process->sp) :); //we load the sp 
	__asm("mov lr, %0": : "r"(current_process->lrUser) :); // we load the lr
	__asm("cps 0b10011");//switch back to svc mode
}

void do_sys_exit(int* pointeurSp)
{
	current_process->status = pointeurSp[1];
	struct pcb_s * saveCurrent = current_process;//we keep the pcb in order to free it at the end of this function
	
	current_process->previousPcb->nextPcb = current_process->nextPcb;
	current_process->nextPcb->previousPcb = current_process->previousPcb;
	
	elect();
	
	//--------------load next process without saving the current one----------------------
	int i = 0;
	for(i=0;i<13;i++)
	{
		// load called process context
		pointeurSp[i] = (uint32_t) current_process->registres[i];
	}
	
	__asm("mov r7, %0": : "r"(current_process->CPSR) :);
	__asm("msr SPSR, r7"); //we load the CPSR
	
	__asm("cps 0b11111");//switch to system mode
	__asm("mov sp, %0": : "r"(current_process->sp) :); //we load the sp 
	__asm("mov lr, %0": : "r"(current_process->lrUser) :); // we load the lr
	__asm("cps 0b10011");//switch back to svc mode
	
	int end = 0;
	if (current_process->nextPcb == current_process)
	{
		end = 1;
	}
	
	//we free datas
	kFree((void*)saveCurrent->sp, 10*1024);
	kFree((void*)saveCurrent, sizeof(struct pcb_s));
	
	if (end==1)
	{
		terminate_kernel();
	}
}

void create_process(func_t* entry)
{
	struct pcb_s * processPcb = (struct pcb_s *) kAlloc(sizeof(struct pcb_s));
	
	processPcb->sp = (uint32_t *)kAlloc(10240) + 10240;
	processPcb->lrUser = (uint32_t) entry;
	processPcb->CPSR = 0b10000;
	
	//We insert the new pcb in the pcbs linked list next to the current_process pcb
	processPcb->nextPcb = current_process->nextPcb;
	processPcb->previousPcb = current_process;
	
	current_process->nextPcb->previousPcb = processPcb;
	current_process->nextPcb = processPcb;
}


void elect()
{
	current_process = current_process->nextPcb;
}


