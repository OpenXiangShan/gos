#include "asm/ptregs.h"
#include "asm/csr.h"
#include "print.h"
#include "timer.h"

#define SCAUSE_IRQ (1UL << 63)

#define INTERRUPT_CAUSE_TIMER       5
#define INTERRUPT_CAUSE_EXTERNAL    9

extern void do_exception_vector(void);

int do_exception(struct pt_regs *regs, unsigned long scause)
{
	if (scause & (1UL << 63)) {
		int cause = scause & (~SCAUSE_IRQ);

		if (cause == INTERRUPT_CAUSE_TIMER)
			irq_do_timer_handler();
	}

	return 0;
}

void trap_init(void)
{
	write_csr(sscratch, 0);

	write_csr(stvec, do_exception_vector);
	myGuest_print("stvec=0x%x, 0x%x\n", read_csr(stvec),
		      do_exception_vector);

	write_csr(sie, -1);
}
