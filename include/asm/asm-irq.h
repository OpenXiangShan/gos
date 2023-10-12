#ifndef _ASM_IRQ_H
#define _ASM_IRQ_H

#include <asm/csr.h>

static inline void __enable_local_irq(void)
{
	csr_set(sstatus, SR_SIE);
}

#endif
