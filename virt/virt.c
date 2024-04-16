#include "virt.h"
#include "mm.h"
#include "print.h"
#include "asm/type.h"
#include "asm/csr.h"
#include "asm/sbi.h"
#include "asm/pgtable.h"
#include "string.h"
#include "machine.h"
#include "cpu_tlb.h"
#include "virt_vm_exit.h"

extern char guest_bin[];

static void enable_gstage_mmu(unsigned long pgdp, int on)
{
	if (!on) {
		write_csr(CSR_HGATP, 0);
	} else {
		write_csr(CSR_HGATP, pgdp >> PAGE_SHIFT | HGATP_MODE);
	}
}

struct vcpu *vcpu_create(void)
{
	struct vcpu *vcpu;
	struct virt_cpu_context *guest_ctx;

	vcpu = (struct vcpu *)mm_alloc(sizeof(struct vcpu));
	if (!vcpu) {
		print("%s -- Out of memory\n", __FUNCTION__);
		return NULL;
	}
	memset((char *)vcpu, 0, sizeof(struct vcpu));

	guest_ctx = &vcpu->cpu_ctx.guest_context;
	guest_ctx->sstatus = SR_SPP | SR_SPIE;
	guest_ctx->hstatus = 0;
	guest_ctx->hstatus |= HSTATUS_VTW;
	guest_ctx->hstatus |= HSTATUS_SPVP;
	guest_ctx->hstatus |= HSTATUS_SPV;

	machine_init(&vcpu->machine);

	return vcpu;
}

int vcpu_run(struct vcpu *vcpu, char *cmd)
{
	struct virt_cpu_context *guest_ctx = &vcpu->cpu_ctx.guest_context;
	extern unsigned long __guest_payload_start;
	extern unsigned long __guest_payload_end;
	int guest_bin_size =
	    (char *)&__guest_payload_end - (char *)&__guest_payload_start,
	    guest_ddr_size, guest_sram_size;
	unsigned long gpa, sram_gpa;
	unsigned long pgdp_va;
	char *guest_bin_ptr = guest_bin;
	int len;

	/* map ddr gstage page table */
	gpa = machine_get_ddr_start(&vcpu->machine);
	guest_ddr_size = machine_get_ddr_size(&vcpu->machine);

	vcpu->host_memory_va = (unsigned long)mm_alloc(guest_ddr_size);
	if (!vcpu->host_memory_va) {
		print("%s -- Out of memory\n", __FUNCTION__);
		return -1;
	}
	vcpu->guest_memory_pa = gpa;
	vcpu->memory_size = guest_ddr_size;
	vcpu->host_memory_pa = virt_to_phy(vcpu->host_memory_va);

	pgdp_va = (unsigned long)mm_alloc(PAGE_SIZE * 4);
	if (!pgdp_va) {
		print("%s -- Out of memory\n");
		return -1;
	}
	vcpu->machine.gstage_pgdp = virt_to_phy(pgdp_va);

	print("gstage page mapping[ddr] -- gpa:0x%lx -> hpa:0x%lx size:0x%x\n",
	      vcpu->guest_memory_pa, vcpu->host_memory_pa, vcpu->memory_size);
	gstage_page_mapping((unsigned long *)vcpu->machine.gstage_pgdp,
			    vcpu->host_memory_pa,
			    vcpu->guest_memory_pa, vcpu->memory_size);

	/* map sram gstage page table */
	sram_gpa = machine_get_sram_start(&vcpu->machine);
	guest_sram_size = machine_get_sram_size(&vcpu->machine);
	vcpu->host_sram_va = (unsigned long)mm_alloc(guest_sram_size);
	if (!vcpu->host_sram_va) {
		print("%s -- Out of memory\n", __FUNCTION__);
		return -1;
	}
	vcpu->guest_sram_pa = sram_gpa;
	vcpu->sram_size = guest_sram_size;
	vcpu->host_sram_pa = virt_to_phy(vcpu->host_sram_va);

	print("gstage page mapping[sram] -- gpa:0x%lx -> hpa:0x%lx size:0x%x\n",
	      vcpu->guest_sram_pa, vcpu->host_sram_pa, vcpu->sram_size);
	gstage_page_mapping((unsigned long *)vcpu->machine.gstage_pgdp,
			    vcpu->host_sram_pa,
			    vcpu->guest_sram_pa, vcpu->sram_size);

	/* machine finialize */
	machine_finialize(&vcpu->machine);

	/* enable gstage mmu */
	enable_gstage_mmu(vcpu->machine.gstage_pgdp, 1);
	__hfence_gvma_all();

	memcpy((char *)vcpu->host_memory_va, guest_bin_ptr, guest_bin_size);
	memcpy((char *)vcpu->host_sram_va,
	       (char *)vcpu->machine.device_entry,
	       vcpu->machine.device_entry_count *
	       sizeof(struct device_init_entry));

	len =
	    vcpu->machine.device_entry_count * sizeof(struct device_init_entry);
	if (cmd)
		strcpy((char *)vcpu->host_sram_va + len, cmd);

	guest_ctx->sepc = vcpu->guest_memory_pa;
	guest_ctx->a0 = vcpu->guest_sram_pa;
	guest_ctx->a1 = vcpu->guest_sram_pa + len;

	while (1) {
		disable_local_irq();

		vcpu_switch_to(&vcpu->cpu_ctx);
		vcpu_process_vm_exit(vcpu);
		
		enable_local_irq();
	}

	return 0;
}
