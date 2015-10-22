#include <inttypes.h>

struct pcb_s
{
	//R0-R12
	uint32_t registres[13];
	uint32_t lrUser;
	uint32_t lrSvc;
	uint32_t * sp;
	uint32_t CPSR;
	
	int status; 
	
	struct pcb_s * nextPcb;
	struct pcb_s * previousPcb;
	
};

typedef int(func_t) (void);

void sys_yieldto(struct pcb_s* dest);

void sys_yield();

void sys_exit(int status);

void do_sys_yieldto(int* pointeurSp);

void do_sys_yield(int* pointeurSp);

void do_sys_exit(int* pointeurSp);

void create_process(func_t* entry);

void sched_init();

void elect();




