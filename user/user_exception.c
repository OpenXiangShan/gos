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

#include "user.h"
#include "stub.h"
#include "asm/csr.h"
#include "print.h"
#include "uaccess.h"
#include "asm/ptregs.h"
#include "asm/sbi.h"
#include "asm/asm-offsets.h"
#include "asm/trap.h"


extern void *const syscall_table[];

typedef long (*syscall_t)(unsigned long, unsigned long,
			  unsigned long, unsigned long,
			  unsigned long, unsigned long, unsigned long);

static int syscall_handler(struct user_cpu_context *regs)
{
	syscall_t fn;
	unsigned long nr_sys = regs->a7;
	unsigned long orig_a0 = regs->a0;

	fn = syscall_table[nr_sys];
	regs->a0 = fn(orig_a0, regs->a1, regs->a2, regs->a3,
		      regs->a4, regs->a5, regs->a6);

	return 0;
}

void show_user_regs(struct pt_regs *regs)
{
	print("cpu_id:%d\n", sbi_get_cpu_id());
	print("sstatus: 0x%lx hstatus : 0x%lx scause : 0x%lx sbadaddr :0x%lx\n",
			regs->sstatus, regs->hstatus, regs->scause, regs->sbadaddr);
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
}

static int do_user_illegal_inst_fault(struct pt_regs *regs, struct user_cpu_context *u_context)
{
	unsigned long badaddr = regs->sbadaddr;
	int ret = -1;

	if (badaddr == BOSC_DEBUG_INST) {
		u_context->sepc += 4;
		ret = 0;
	}

	return ret;
}

int do_user_exception(struct user *user, struct pt_regs *regs)
{
	struct user_cpu_context *u_context = &user->cpu_context.u_context;
	int ret = 0;

	memcpy((char *)regs, (char *)u_context, sizeof(struct user_cpu_context));
	regs->sepc = u_context->sepc;
	regs->sstatus = u_context->sstatus;
	regs->hstatus = u_context->hstatus;
	regs->scause = read_csr(CSR_SCAUSE);
	regs->sbadaddr = read_csr(CSR_STVAL);

	if (!(regs->scause & (1UL << 63))) {
		switch (regs->scause) {
		case EXC_SYSCALL:
			ret = syscall_handler(u_context);
			u_context->sepc += 4;
			break;
		case EXC_INST_ILLEGAL:
			ret = do_user_illegal_inst_fault(regs, u_context);
			u_context->sepc += 4;
			break;
		default:
			show_user_regs(regs);
			ret = -1;
			break;
		}
	}
	return ret;
}
