#ifndef	_SBI_TRAP_H
#define	_SBI_TRAP_H

#include "asm/trap.h"

#define MCAUSE_IRQ (1UL << 63)

#define IRQ_S_SOFT  1
#define IRQ_M_SOFT  3
#define IRQ_S_TIMER 5
#define IRQ_S_EXT   9
#define IRQ_M_TIMER 7
#define IRQ_M_EXT   11
#define IRQ_S_GEXT   12

#define MIP_SSIP  (1UL << IRQ_S_SOFT)
#define MIP_STIP  (1UL << IRQ_S_TIMER)
#define MIP_SEIP  (1UL << IRQ_S_EXT)
#define MIP_MTIP  (1UL << IRQ_M_TIMER)
#define MIP_MEIP  (1UL << IRQ_M_EXT)
#define MIP_MSIP  (1UL << IRQ_M_SOFT)
#define MIP_SGEIP  (1UL << IRQ_S_GEXT)

#define M_MODE 0
#define S_MODE 1

struct sbi_trap_regs {
	unsigned long mepc;
	unsigned long ra;
	unsigned long sp;
	unsigned long gp;
	unsigned long tp;
	unsigned long t0;
	unsigned long t1;
	unsigned long t2;
	unsigned long s0;
	unsigned long s1;
	unsigned long a0;
	unsigned long a1;
	unsigned long a2;
	unsigned long a3;
	unsigned long a4;
	unsigned long a5;
	unsigned long a6;
	unsigned long a7;
	unsigned long s2;
	unsigned long s3;
	unsigned long s4;
	unsigned long s5;
	unsigned long s6;
	unsigned long s7;
	unsigned long s8;
	unsigned long s9;
	unsigned long s10;
	unsigned long s11;
	unsigned long t3;
	unsigned long t4;
	unsigned long t5;
	unsigned long t6;
	/* mstatus register state */
	unsigned long mstatus;
};

struct sbi_trap_hw_context {
	unsigned int hart_id;
	/* clint */
	unsigned long uart_base;
	unsigned long timer_cmp;
	unsigned long ipi;
	/* imsic */
	unsigned long imsic_base;
	unsigned long imsic_ids_used_bits[2048 / (sizeof(unsigned long) * 8)];
	int imsic_nr_ids;
	/* context */
	unsigned long next_addr;
	char wait_var;
	char next_mode[16];
	unsigned long hw_info;
};

void sbi_trap_handler(struct sbi_trap_regs *regs);
void delegate_traps();

#endif
