#include <inttypes.h>


void swi_handler(void);
void irq_handler(void);

void sys_reboot();
void sys_nop();
void sys_settime(uint64_t date_ms);
uint64_t sys_gettime();

void do_sys_reboot();
void do_sys_nop();
void do_sys_settime(int* pointeurSp);
void do_sys_gettime(int* pointeurSp);

