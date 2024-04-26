#ifndef __TRAP_H__
#define __TRAP_H__

#include "asm/ptregs.h"

int do_exception(struct pt_regs *regs, unsigned long scause);
void trap_init(void);

#endif
