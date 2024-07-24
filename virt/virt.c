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
#include "vcpu_timer.h"
#include "uapi/align.h"
#include "vcpu_aia.h"
#include "gos.h"
#include "list.h"
#include "percpu.h"

extern char guest_bin[];
static DEFINE_PER_CPU(struct list_head, vcpu_list);
static DEFINE_PER_CPU(unsigned long, vmid_bitmap);

static void vcpu_vmid_init(void)
{
	int cpu;
	unsigned long *vmid;

	for_each_online_cpu(cpu) {
		vmid = &per_cpu(vmid_bitmap, cpu);
		*vmid = 1;
	}
}

static int find_free_vmid(unsigned long *vmid)
{
	unsigned long bitmap = *vmid;
	int pos = 0;

	while (bitmap & 0x01) {
		if (pos == 64)
			return -1;
		bitmap = bitmap >> 1;
		pos++;
	}

	*vmid |= (1UL) << pos;

	return pos;
}

static int vcpu_alloc_vmid(int cpu)
{
	unsigned long *vmid;

	vmid = &per_cpu(vmid_bitmap, cpu);
	if (!vmid)
		return -1;

	return find_free_vmid(vmid);
}

static void vcpu_update_vmid(struct vcpu *vcpu)
{
	vcpu->vmid = vcpu_alloc_vmid(sbi_get_cpu_id());
}

static void vcpu_update(struct vcpu *vcpu)
{
	vcpu->last_cpu = vcpu->cpu;
	vcpu->cpu = sbi_get_cpu_id();
}

static void vcpu_save(struct vcpu *vcpu)
{
	struct cpu_context *ctx = &vcpu->cpu_ctx;

	ctx->vsscratch = read_csr(CSR_VSSCRATCH);
	ctx->vsstatus = read_csr(CSR_VSSTATUS);
	ctx->vstvec = read_csr(CSR_VSTVEC);
	ctx->vstval = read_csr(CSR_VSTVAL);
	ctx->vsie = read_csr(CSR_VSIE);
	ctx->vsip = read_csr(CSR_VSIP);
	ctx->vsatp = read_csr(CSR_VSATP);
	ctx->vsepc = read_csr(CSR_VSEPC);
	ctx->hvip = read_csr(CSR_HVIP);
}

static void vcpu_restore(struct vcpu *vcpu)
{
	struct cpu_context *ctx = &vcpu->cpu_ctx;

	write_csr(CSR_VSSCRATCH, ctx->vsscratch);
	write_csr(CSR_VSSTATUS, ctx->vsstatus);
	write_csr(CSR_VSTVEC, ctx->vstvec);
	write_csr(CSR_VSTVAL, ctx->vstval);
	write_csr(CSR_VSIE, ctx->vsie);
	write_csr(CSR_VSIP, ctx->vsip);
	write_csr(CSR_VSATP, ctx->vsatp);
	write_csr(CSR_VSEPC, ctx->vsepc);
	write_csr(CSR_HVIP, ctx->hvip);
}

static void enable_gstage_mmu(unsigned long pgdp, int on, int vmid)
{
	if (!on) {
		write_csr(CSR_HGATP, 0);
	} else {
		write_csr(CSR_HGATP,
			  pgdp >> PAGE_SHIFT |
			  HGATP_MODE |
			  HGATP_VMID(vmid));
	}
}

static void update_hgatp(struct vcpu *vcpu)
{
	write_csr(CSR_HGATP,
		  (vcpu->machine.gstage_pgdp >> PAGE_SHIFT) |
		  HGATP_MODE |
		  HGATP_VMID(vcpu->vmid));

	__hfence_gvma_all();
}

static void vcpu_fence_gvma_all()
{
	__hfence_gvma_all();
}

static void vcpu_inject_interrupt(struct vcpu *vcpu, int irq)
{
	struct cpu_context *ctx = &vcpu->cpu_ctx;

	ctx->hvip |= (1UL) << irq;
}

static void vcpu_do_interrupt(struct vcpu *vcpu)
{
	if (!vcpu->irq_pending) {
		vcpu->cpu_ctx.hvip = 0;
		return;
	}

	if (vcpu_check_irq_pending(vcpu->irq_pending, IRQ_VS_TIMER))
		vcpu_inject_interrupt(vcpu, IRQ_VS_TIMER);
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

static int vcpu_create_gstage_mapping(struct vcpu *vcpu)
{
	int guest_ddr_size, guest_sram_size, guest_memory_test_size;
	unsigned long gpa, sram_gpa, memory_test_gpa;
	unsigned long pgdp_va;

	/* map ddr gstage page table */
	gpa = machine_get_ddr_start(&vcpu->machine);
	guest_ddr_size = machine_get_ddr_size(&vcpu->machine);

#if CONFIG_SELECT_4K_GUEST_MEM_MAPPING
	vcpu->host_memory_va = (unsigned long)mm_alloc(guest_ddr_size);
#elif CONFIG_SELECT_2M_GUEST_MEM_MAPPING
	vcpu->host_memory_va =
	    (unsigned long)mm_alloc_align(2 * 1024 * 1024, guest_ddr_size);
	print("%s -- vcpu->host_memory_va: 0x%lx\n", __FUNCTION__,
	      vcpu->host_memory_va);
#elif CONFIG_SELECT_1G_GUEST_MEM_MAPPING
	vcpu->host_memory_va =
	    (unsigned long)mm_alloc_align(1 * 1024 * 1024 * 1024,
					  guest_ddr_size);
#endif
	if (!vcpu->host_memory_va) {
		print("%s -- Out of memory\n", __FUNCTION__);
		return -1;
	}
	vcpu->guest_memory_pa = gpa;
	vcpu->memory_size = guest_ddr_size;
	vcpu->host_memory_pa = virt_to_phy(vcpu->host_memory_va);

	pgdp_va = (unsigned long)mm_alloc_align(PAGE_SIZE * 4, PAGE_SIZE * 4);
	if (!pgdp_va) {
		print("%s -- Out of memory\n");
		return -1;
	}
	memset((char *)pgdp_va, 0, PAGE_SIZE * 4);
	vcpu->machine.gstage_pgdp = virt_to_phy(pgdp_va);

	print("gstage page mapping[ddr] -- gpa:0x%lx -> hpa:0x%lx size:0x%x\n",
	      vcpu->guest_memory_pa, vcpu->host_memory_pa, vcpu->memory_size);
#if CONFIG_SELECT_4K_GUEST_MEM_MAPPING
	gstage_page_mapping((unsigned long *)vcpu->machine.gstage_pgdp,
			    vcpu->host_memory_pa,
			    vcpu->guest_memory_pa, vcpu->memory_size);
#elif CONFIG_SELECT_2M_GUEST_MEM_MAPPING
	gstage_page_mapping_2M((unsigned long *)vcpu->machine.gstage_pgdp,
			       vcpu->host_memory_pa,
			       vcpu->guest_memory_pa, vcpu->memory_size);
#elif CONFIG_SELECT_1G_GUEST_MEM_MAPPING
	gstage_page_mapping_1G((unsigned long *)vcpu->machine.gstage_pgdp,
			       vcpu->host_memory_pa,
			       vcpu->guest_memory_pa, vcpu->memory_size);
#endif

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

	return 0;
}

static int vcpu_load_guest_running_data(struct vcpu *vcpu,
					struct virt_run_params *params)
{
	struct virt_cpu_context *guest_ctx = &vcpu->cpu_ctx.guest_context;
	extern unsigned long __guest_payload_start;
	extern unsigned long __guest_payload_end;
	int guest_bin_size =
	    (char *)&__guest_payload_end - (char *)&__guest_payload_start;
	int len;
	void *ptr;
	char *guest_bin_ptr = guest_bin;

	/* copy guest_bin */
	memcpy((char *)vcpu->host_memory_va, guest_bin_ptr, guest_bin_size);
	guest_ctx->sepc = vcpu->guest_memory_pa;	//start of myGuest

	/* copy relevant structure to sram */
	ptr = (void *)vcpu->host_sram_va;

	// 1. copy device_entry to sram
	memcpy(ptr, (char *)vcpu->machine.device_entry,
	       vcpu->machine.device_entry_count *
	       sizeof(struct device_init_entry));
	len =
	    vcpu->machine.device_entry_count * sizeof(struct device_init_entry);
	guest_ctx->a0 = vcpu->guest_sram_pa + (unsigned long)(ptr - vcpu->host_sram_va);	//myGuest a0 --> device_entry
	vcpu->machine.device_entry_host_va = (struct device_init_entry *)ptr;
	ptr += len;
	ptr = PTR_ALIGN(ptr, 8);

	// 2. copy run_params to sram
	if (params) {
		params->vmid = vcpu->vmid;
		params->cpu = sbi_get_cpu_id();
		memcpy((char *)ptr, (void *)params,
		       sizeof(struct virt_run_params));
		vcpu->run_params = (struct virt_run_params *)ptr;
		vcpu->run_params->busy = 1;
	}
	len = sizeof(struct virt_run_params);
	guest_ctx->a1 = vcpu->guest_sram_pa + (unsigned long)(ptr - vcpu->host_sram_va);	//myGuest a1 --> run_params
	ptr += len;
	ptr = PTR_ALIGN(ptr, 8);

	// 3. copy device_entry_data to sram
	vcpu->machine.entry_data_gpa =
	    (void *)(vcpu->guest_sram_pa + (ptr - vcpu->host_sram_va));
	device_entry_data_redirect(&vcpu->machine);
	len = vcpu->machine.entry_data_len;
	memcpy((char *)ptr, (char *)vcpu->machine.entry_data, len);
	ptr += len;
	ptr = PTR_ALIGN(ptr, 8);

	return 0;
}

static int vcpu_exception_delegation(void)
{
	unsigned long hideleg, hedeleg;

	hedeleg = 0;
	hedeleg |= (1UL << EXC_INST_MISALIGNED);
	hedeleg |= (1UL << EXC_INST_ILLEGAL);
	hedeleg |= (1UL << EXC_BREAKPOINT);
	hedeleg |= (1UL << EXC_LOAD_MISALIGNED);
	hedeleg |= (1UL << EXC_STORE_MISALIGNED);
	hedeleg |= (1UL << EXC_SYSCALL);
	hedeleg |= (1UL << EXC_INST_PAGE_FAULT);
	hedeleg |= (1UL << EXC_LOAD_PAGE_FAULT);
	hedeleg |= (1UL << EXC_STORE_PAGE_FAULT);
	write_csr(CSR_HEDELEG, hedeleg);

	hideleg = 0;
	hideleg |= (1UL << IRQ_VS_SOFT);
	hideleg |= (1UL << IRQ_VS_TIMER);
	hideleg |= (1UL << IRQ_VS_EXT);
	write_csr(CSR_HIDELEG, hideleg);

	write_csr(CSR_HCOUNTEREN, 0x02);

	write_csr(CSR_HVIP, 0);

	return 0;
}

void vcpu_set_request(struct vcpu *vcpu, unsigned int req)
{
	vcpu->request |= ((1UL) << req);
}

struct vcpu *get_vcpu(int vmid, int cpu)
{
	struct vcpu *vcpu;
	struct list_head *vcpus;

	vcpus = &per_cpu(vcpu_list, cpu);

	list_for_each_entry(vcpu, vcpus, list) {
		if (vcpu->vmid == vmid)
			return vcpu;
	}

	return NULL;
}

static void __dump_vcpu_info(int cpu)
{
	struct vcpu *vcpu;
	struct list_head *vcpus;

	vcpus = &per_cpu(vcpu_list, cpu);
	if (!vcpus) {
		print("invalid hart id: %d\n", cpu);
		return;
	}

	print("+++++++++++++ vcpu info on cpu%d +++++++++++++\n", cpu);
	list_for_each_entry(vcpu, vcpus, list) {
		print("@@@@@@@@@@@@@@ VM%d info: @@@@@@@@@@@@@@\n", vcpu->vmid);
		print("- vmid : %d\n", vcpu->vmid);
		print("- memory info:\n");
		print("    host_memory_va : 0x%lx\n", vcpu->host_memory_va);
		print("    host_memory_pa : 0x%lx\n", vcpu->host_memory_pa);
		print("    guest_memory_pa : 0x%lx\n", vcpu->guest_memory_pa);
		print("    memory_size : 0x%x\n", vcpu->memory_size);
		print("- memory test info:\n");
		print("    host_memory_test_va : 0x%lx\n", vcpu->host_memory_test_va);
		print("    host_memory_test_pa : 0x%lx\n", vcpu->host_memory_test_pa);
		print("    guest_memory_test_pa : 0x%lx\n", vcpu->guest_memory_test_pa);
		print("    memory_test_size : 0x%x\n", vcpu->memory_test_size);
#if CONFIG_VIRT_ENABLE_AIA
		print("- aia info:\n");
		print("    hgei : %d\n", vcpu->hgei);
		print("    vs_interrupt_file_hva : 0x%lx\n", vcpu->vs_interrupt_file_va);
		print("    vs_interrupt_file_hpa : 0x%lx\n", vcpu->vs_interrupt_file_pa);
		print("    vs_interrupt_file_gpa : 0x%lx\n", vcpu->vs_interrupt_file_gpa);
		print("    vs_interrupt_file_size : 0x%x\n", vcpu->vs_interrupt_file_size);
#endif
		print("\n");
	}
}

void dump_vcpu_info_on_all_cpu(void)
{
	int cpu;

	for_each_online_cpu(cpu)
		__dump_vcpu_info(cpu);
}

void dump_vcpu_info_on_cpu(int cpu)
{
	__dump_vcpu_info(cpu);
}

static struct vcpu *__vcpu_create(void)
{
	struct vcpu *vcpu;
	struct virt_cpu_context *guest_ctx;
	struct list_head *vcpus;

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
	if (read_csr(sstatus) & SR_FS) {
		vcpu->cpu_ctx.vsstatus |= SR_FS;
		guest_ctx->sstatus |= SR_FS;
	}

	machine_init(&vcpu->machine);

#if CONFIG_VIRT_ENABLE_TIMER
	vcpu_timer_init(vcpu);
#endif
	vcpu->cpu = -1;

	vcpus = &per_cpu(vcpu_list, sbi_get_cpu_id());
	list_add_tail(&vcpu->list, vcpus);

	return vcpu;
}

struct vcpu *vcpu_create_force(void)
{
	return __vcpu_create();
}

struct vcpu *vcpu_create(void)
{
	struct vcpu *vcpu;
	struct list_head *vcpus;

	vcpus = &per_cpu(vcpu_list, sbi_get_cpu_id());

	if (list_empty(vcpus))
		goto create_vcpu;

	vcpu = list_entry(list_first(vcpus), struct vcpu, list);
	if (vcpu)
		return vcpu;

create_vcpu:
	return __vcpu_create();
}

int vcpu_run(struct vcpu *vcpu, struct virt_run_params *params)
{
	unsigned long start, end;

	start = sbi_get_cpu_cycles();

	if (vcpu->running == 1) {
		return vcpu_set_run_params(vcpu, params);
	}

	if (vcpu_create_gstage_mapping(vcpu)) {
		print("vcpu create gstage failed...\n");
		return -1;
	}

	vcpu_update_vmid(vcpu);
	print("vmid: %d\n", vcpu->vmid);

	enable_gstage_mmu(vcpu->machine.gstage_pgdp, 1, vcpu->vmid);
	__hfence_gvma_all();

	vcpu_load_guest_running_data(vcpu, params);

	vcpu_exception_delegation();

	machine_finialize(&vcpu->machine);

#if CONFIG_VIRT_ENABLE_TIMER
	vcpu_time_init(vcpu);
#endif
	vcpu->running = 1;

	end = sbi_get_cpu_cycles();
	print("vcpu startup success, cost: %d(cycles)\n", end - start);
	while (1) {
		vcpu_update_run_params(vcpu);

		disable_local_irq();

		vcpu_update(vcpu);

		vcpu_do_request(vcpu);

		vcpu_do_interrupt(vcpu);

#if CONFIG_VIRT_ENABLE_AIA
		vcpu_interrupt_file_update(vcpu);
#endif
		vcpu_restore(vcpu);

		update_hgatp(vcpu);

		vcpu_switch_to(&vcpu->cpu_ctx);

		if (vcpu_process_vm_exit(vcpu) == -1) {
			enable_local_irq();
			vcpu->running = 0;
			break;
		}

		vcpu_save(vcpu);

		enable_local_irq();
	}

	return 0;
}

void vcpu_init(void)
{
	int cpu;
	struct list_head *vcpus;

	for_each_online_cpu(cpu) {
		vcpus = &per_cpu(vcpu_list, cpu);
		INIT_LIST_HEAD(vcpus);
	}

	vcpu_vmid_init();

#if CONFIG_VIRT_ENABLE_AIA
	vcpu_aia_init();
#endif
}
