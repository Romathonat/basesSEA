# include "util.h" 
# include "syscall.h"
# include "sched.h"

struct pcb_s pcb1, pcb2; 
struct pcb_s *p1, *p2; 

void user_process_1() 
{ 
	int v1=5;
	while (1)
	{ 
		v1++; 
		sys_yieldto(p2); 
	} 
} 

void user_process_2() 
{ 
	int v2=-12; 
	while (1) 
	{ 
		v2-=2; 
		sys_yieldto(p1); 
	} 
}

void kmain( void ) 
{ 
	sched_init(); 
	
	/*Needed before managing different stacks
	p1=&pcb1; 
	p2=&pcb2; //initialize p1 and p2//
	//initialize lr for first time processus switch
	p1->lrUser= (uint32_t) &user_process_1;
	p2->lrUser= (uint32_t) &user_process_2;
	*/
	
	p1=create_process((func_t*) &user_process_1);
	p2=create_process((func_t*) &user_process_2);
	
	__asm("cps 0x10");//switch CPU to USER mode//


	sys_yieldto(p1);
	
	// this is now unreachable

	PANIC();
}



/*int kmain(void)
{
	//switch to user mode
	__asm("cps 0x10");
	
	uint64_t date_ms = 0x12345678CACACACA;
	sys_settime(date_ms);

	
	//__asm("mrs r0, SPSR");
	//__asm("cps #0b10000");
	//__asm("cps #0b10011");
	//__asm("bl dummy");
	int radius = 5;
	//__asm("mov r2, %0" : : "r"(radius) : "r3", "r2");
	//__asm("mov %0, r3" : "=r"(radius) : : "r3", "r2");
	int volume;
	dummy();
	volume = compute_volume(radius);
	return volume;
	return 0;
}*/
