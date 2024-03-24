#ifndef _ASM_IRQ_H
#define _ASM_IRQ_H

#include <asm/csr.h>

#define local_irq_save(flags) \
	(flags) = csr_read_clear(sstatus, SR_SIE);

#define local_irq_restore(flags) \
	csr_set(sstatus, (flags) & SR_SIE);

static inline void __enable_local_irq(void)
{
	csr_set(sstatus, SR_SIE);
}

#endif
