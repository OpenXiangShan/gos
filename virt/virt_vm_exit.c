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

#include "virt.h"
#include "asm/csr.h"
#include "print.h"
#include "machine.h"
#include "vcpu_insn.h"
#include "vcpu_sbi.h"

static void panic()
{
	print("Kernel panic\n");
}


void show_guest_regs(struct virt_cpu_context *regs)
{
	print("sepc: 0x%lx ra : 0x%lx sp : 0x%lx\n", regs->sepc, regs->ra, regs->sp);
	print(" gp : 0x%lx tp : 0x%lx t0 : 0x%lx\n", regs->gp, regs->tp, regs->t0);
	print(" t1 : 0x%lx t2 : 0x%lx t3 : 0x%lx\n", regs->t1, regs->t2, regs->s0);
	print(" s1 : 0x%lx a0 : 0x%lx a1 : 0x%lx\n", regs->s1, regs->a0, regs->a1);
	print(" a2 : 0x%lx a3 : 0x%lx a4 : 0x%lx\n", regs->a2, regs->a3, regs->a4);
	print(" a5 : 0x%lx a6 : 0x%lx a7 : 0x%lx\n", regs->a5, regs->a6, regs->a7);
	print(" s2 : 0x%lx s3 : 0x%lx s4 : 0x%lx\n", regs->s2, regs->s3, regs->s4);
	print(" s5 : 0x%lx s6 : 0x%lx s7 : 0x%lx\n", regs->s5, regs->s6, regs->s7);
	print(" s8 : 0x%lx s9 : 0x%lx s10: 0x%lx\n", regs->s8, regs->s9, regs->s10);
	print(" s11: 0x%lx t3 : 0x%lx t4: 0x%lx\n", regs->s11, regs->t3, regs->t4);
	print(" t5 : 0x%lx t6 : 0x%lx\n", regs->t5, regs->t6);
}

static void vcpu_do_trap_error(struct vcpu *vcpu, unsigned long saus)
{
	struct virt_cpu_context *regs;

	if (saus == EXC_LOAD_GUEST_PAGE_FAULT)
		print("Oops - %s\n", "EXC_LOAD_GUEST_PAGE_FAULT");
	else if (saus == EXC_STORE_GUEST_PAGE_FAULT)
		print("Oops - %s\n", "EXC_STORE_GUEST_PAGE_FAULT");

	regs = &(vcpu->cpu_ctx.guest_context);
	show_guest_regs(regs);

	print("vsepc: 0x%lx  vstatus:0x%lx  vsatp:0x%lx\n", vcpu->cpu_ctx.vsepc,
			vcpu->cpu_ctx.vsstatus, vcpu->cpu_ctx.vsatp);
	print("htval: 0x%lx  stval:0x%lx\n", read_csr(CSR_HTVAL), read_csr(CSR_STVAL));
	panic();
}

static int gstage_page_fault(struct vcpu *vcpu, unsigned long reason)
{
	unsigned long fault_addr;
	unsigned long htval, stval;
	unsigned long htinst;
	int ret = -1;

	htval = read_csr(CSR_HTVAL);
	stval = read_csr(CSR_STVAL);
	htinst = read_csr(CSR_HTINST);

	fault_addr = (htval << 2) | (stval & 0x3);
	//print("%s %d fault_addr:0x%lx htinst:0x%lx\n", __FUNCTION__, __LINE__,
	//      fault_addr, htinst);

	switch (reason) {
	case EXC_LOAD_GUEST_PAGE_FAULT:
		ret = vcpu_mmio_load(vcpu, fault_addr, htinst);
		break;
	case EXC_STORE_GUEST_PAGE_FAULT:
		ret = vcpu_mmio_store(vcpu, fault_addr, htinst);
		break;
	default:
		break;
	}

	return ret;
}

int vcpu_process_vm_exit(struct vcpu *vcpu)
{
	struct virt_cpu_context *guest_ctx;
	unsigned long scause;

	guest_ctx = &vcpu->cpu_ctx.guest_context;

	scause = read_csr(CSR_SCAUSE);

	switch (scause) {
	case EXC_INST_ILLEGAL:
	case EXC_LOAD_MISALIGNED:
	case EXC_STORE_MISALIGNED:
		break;
	case EXC_VIRTUAL_INST_FAULT:
		break;
	case EXC_INST_GUEST_PAGE_FAULT:
	case EXC_LOAD_GUEST_PAGE_FAULT:
	case EXC_STORE_GUEST_PAGE_FAULT:
		if (guest_ctx->hstatus & HSTATUS_SPV) {
			if(gstage_page_fault(vcpu, scause)){
				vcpu_do_trap_error(vcpu, scause);
				return -1;
			}
		}else
			print
			    ("%s -- hstatus state error! hstatus:0x%lx\n cause:0x%x",
			     guest_ctx->hstatus, scause);
		break;
	case EXC_SUPERVISOR_SYSCALL:
		if (guest_ctx->hstatus & HSTATUS_SPV)
			vcpu_sbi_call(vcpu);
		else
			print
			    ("%s -- hstatus state error! hstatus:0x%lx\n cause:0x%x",
			     guest_ctx->hstatus, scause);
		break;
	default:
		break;

	}
	return 0;
}
