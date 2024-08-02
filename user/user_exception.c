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

int do_user_exception(struct user *user, struct pt_regs *regs)
{
	struct user_cpu_context *u_context = &user->cpu_context.u_context;
	struct fault_info *fi;

        memcpy((char *)regs, (char *)u_context, sizeof(struct user_cpu_context));
        regs->sepc = u_context->sepc;
        regs->sstatus = u_context->sstatus;
        regs->hstatus = u_context->hstatus;
        regs->scause = read_csr(CSR_SCAUSE);
        regs->sbadaddr = read_csr(CSR_STVAL);

	if (!(regs->scause & (1UL << 63))) {
		switch (regs->scause) {
		case EXC_SYSCALL:
			syscall_handler(u_context);
			u_context->sepc += 4;
			break;
		default:
			fi = ec_to_fault_info(regs->scause);
			if (fi)
				fi->fn(regs, fi->name);
			break;
		}
	}
	return 0;
}
