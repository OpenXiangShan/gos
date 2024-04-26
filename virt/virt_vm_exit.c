#include "virt.h"
#include "asm/csr.h"
#include "print.h"
#include "machine.h"
#include "vcpu_insn.h"
#include "vcpu_sbi.h"

static int gstage_page_fault(struct vcpu *vcpu, unsigned long reason)
{
	unsigned long fault_addr;
	unsigned long htval, stval;
	unsigned long htinst;

	htval = read_csr(CSR_HTVAL);
	stval = read_csr(CSR_STVAL);
	htinst = read_csr(CSR_HTINST);

	fault_addr = (htval << 2) | (stval & 0x3);
	//print("%s %d fault_addr:0x%lx htinst:0x%lx\n", __FUNCTION__, __LINE__,
	//      fault_addr, htinst);

	switch (reason) {
	case EXC_LOAD_GUEST_PAGE_FAULT:
		vcpu_mmio_load(vcpu, fault_addr, htinst);
		return 1;
	case EXC_STORE_GUEST_PAGE_FAULT:
		vcpu_mmio_store(vcpu, fault_addr, htinst);
		return 1;
	}

	return 0;
}

void vcpu_process_vm_exit(struct vcpu *vcpu)
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
		if (guest_ctx->hstatus & HSTATUS_SPV)
			gstage_page_fault(vcpu, scause);
		else
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
}
