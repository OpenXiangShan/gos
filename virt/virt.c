#include "virt.h"
#include "mm.h"
#include "print.h"
#include "asm/type.h"
#include "asm/csr.h"
#include "asm/sbi.h"
#include "asm/pgtable.h"
#include "asm/barrier.h"
#include "string.h"
#include "machine.h"
#include "cpu_tlb.h"
#include "virt_vm_exit.h"

extern char guest_bin[];
static struct vcpu *p_vcpu __attribute__((section(".data"))) = NULL;

static void enable_gstage_mmu(unsigned long pgdp, int on)
{
	if (!on) {
		write_csr(CSR_HGATP, 0);
	} else {
		write_csr(CSR_HGATP, pgdp >> PAGE_SHIFT | HGATP_MODE);
	}
}

static void vcpu_fence_gvma_all()
{
	__hfence_gvma_all();
}

static void vcpu_do_request(struct vcpu *vcpu)
{
	if (!vcpu->request)
		return;

	if (vcpu_check_request(vcpu->request, VCPU_REQ_FENCE_GVMA_ALL))
		vcpu_fence_gvma_all();
}

static void vcpu_update_run_params(struct vcpu *vcpu)
{
	struct virt_run_params *params = vcpu->run_params;
	struct virt_run_params *host_params = &vcpu->host_run_params;

	if (!params)
		return;

	if (params->busy == 1)
		return;

	if (host_params->busy == 1) {
		__smp_rmb();
		memcpy((char *)params, (char *)host_params,
		       sizeof(struct virt_run_params));
		__smp_wmb();
		params->busy = 1;
		host_params->busy = 0;
	}
}

static int vcpu_set_run_params(struct vcpu *vcpu, struct virt_run_params *cmd)
{
	struct virt_run_params *params = &vcpu->host_run_params;

	while (params->busy == 1) ;

	__smp_rmb();

	memcpy((char *)params, (char *)cmd, sizeof(struct virt_run_params));

	__smp_wmb();

	params->busy = 1;

	return 0;
}

void vcpu_set_request(struct vcpu *vcpu, unsigned int req)
{
	vcpu->request |= ((1UL) << req);
}

struct vcpu *vcpu_create(void)
{
	struct vcpu *vcpu;
	struct virt_cpu_context *guest_ctx;

	if (p_vcpu)
		return p_vcpu;

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

	p_vcpu = vcpu;

	return vcpu;
}

int vcpu_run(struct vcpu *vcpu, struct virt_run_params *params)
{
	struct virt_cpu_context *guest_ctx = &vcpu->cpu_ctx.guest_context;
	extern unsigned long __guest_payload_start;
	extern unsigned long __guest_payload_end;
	int guest_bin_size =
	    (char *)&__guest_payload_end - (char *)&__guest_payload_start,
	    guest_ddr_size, guest_sram_size, guest_memory_test_size;
	unsigned long gpa, sram_gpa, memory_test_gpa;
	unsigned long pgdp_va;
	char *guest_bin_ptr = guest_bin;
	int len;

	if (vcpu->running == 1) {
		return vcpu_set_run_params(vcpu, params);
	}

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

	/* map memory test gstage page table */
	memory_test_gpa = machine_get_memory_test_start(&vcpu->machine);
	guest_memory_test_size = machine_get_memory_test_size(&vcpu->machine);
	vcpu->host_memory_test_va =
	    (unsigned long)mm_alloc(guest_memory_test_size);
	if (!vcpu->host_memory_test_va) {
		print("%s -- Out of memory\n", __FUNCTION__);
		return -1;
	}
	strcpy((char *)vcpu->host_memory_test_va,
	       "Hello this is memory test pa1!!!");
	vcpu->guest_memory_test_pa = memory_test_gpa;
	vcpu->host_memory_test_pa = virt_to_phy(vcpu->host_memory_test_va);
	vcpu->memory_test_size = guest_memory_test_size;

	print
	    ("gstage page mapping[memory test] -- gpa:0x%lx -> hpa:0x%lx size:0x%x\n",
	     vcpu->guest_memory_test_pa, vcpu->host_memory_test_pa,
	     vcpu->memory_test_size);
	gstage_page_mapping((unsigned long *)vcpu->machine.gstage_pgdp,
			    vcpu->host_memory_test_pa,
			    vcpu->guest_memory_test_pa, vcpu->memory_test_size);

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
	if (params) {
		strcpy((char *)vcpu->host_sram_va + len, (char *)params);
		vcpu->run_params =
		    (struct virt_run_params *)(vcpu->host_sram_va + len);
		vcpu->run_params->busy = 1;
	}

	guest_ctx->sepc = vcpu->guest_memory_pa;
	guest_ctx->a0 = vcpu->guest_sram_pa;
	guest_ctx->a1 = vcpu->guest_sram_pa + len;

	vcpu->running = 1;

	while (1) {
		vcpu_do_request(vcpu);

		vcpu_update_run_params(vcpu);

		disable_local_irq();

		vcpu_switch_to(&vcpu->cpu_ctx);
		vcpu_process_vm_exit(vcpu);

		enable_local_irq();
	}

	return 0;
}
