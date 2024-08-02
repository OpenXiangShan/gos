/*
 * Copyright (c) 2024 Beijing Institute of Open Source Chip (BOSC)
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2 or later, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "asm/ptregs.h"
#include "asm/csr.h"
#include "print.h"
#include "timer.h"
#include "irq.h"
#include "mm.h"

#define SCAUSE_IRQ (1UL << 63)

#define INTERRUPT_CAUSE_TIMER       5
#define INTERRUPT_CAUSE_EXTERNAL    9

extern void do_exception_vector(void);

static void show_regs(struct pt_regs *regs)
{
	print("sepc: 0x%lx ra : 0x%lx sp : 0x%lx\n", regs->sepc, regs->ra,
	      regs->sp);
	print(" gp : 0x%lx tp : 0x%lx t0 : 0x%lx\n", regs->gp, regs->tp,
	      regs->t0);
	print(" t1 : 0x%lx t2 : 0x%lx t3 : 0x%lx\n", regs->t1, regs->t2,
	      regs->s0);
	print(" s1 : 0x%lx a0 : 0x%lx a1 : 0x%lx\n", regs->s1, regs->a0,
	      regs->a1);
	print(" a2 : 0x%lx a3 : 0x%lx a4 : 0x%lx\n", regs->a2, regs->a3,
	      regs->a4);
	print(" a5 : 0x%lx a6 : 0x%lx a7 : 0x%lx\n", regs->a5, regs->a6,
	      regs->a7);
	print(" s2 : 0x%lx s3 : 0x%lx s4 : 0x%lx\n", regs->s2, regs->s3,
	      regs->s4);
	print(" s5 : 0x%lx s6 : 0x%lx s7 : 0x%lx\n", regs->s5, regs->s6,
	      regs->s7);
	print(" s8 : 0x%lx s9 : 0x%lx s10: 0x%lx\n", regs->s8, regs->s9,
	      regs->s10);
	print(" s11: 0x%lx t3 : 0x%lx t4: 0x%lx\n", regs->s11, regs->t3,
	      regs->t4);
	print(" t5 : 0x%lx t6 : 0x%lx\n", regs->t5, regs->t6);
	print("stvec : 0x%x\n", read_csr(stvec));
	print("stval : 0x%x\n", read_csr(stval));
}

int do_exception(struct pt_regs *regs, unsigned long scause)
{
	if (scause & (1UL << 63)) {
		int cause = scause & (~SCAUSE_IRQ);

		if (cause == INTERRUPT_CAUSE_TIMER)
			irq_do_timer_handler();
		else if (cause == INTERRUPT_CAUSE_EXTERNAL)
			irq_handler();
	} else {
		print("scause:0x%lx\n", scause);
		if (scause == EXC_STORE_PAGE_FAULT ||
		    scause == EXC_LOAD_PAGE_FAULT)
			dump_fault_addr_pt(read_csr(stval));
		show_regs(regs);
		while(1);
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
