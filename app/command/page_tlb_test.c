#include <asm/type.h>
#include <print.h>
#include <device.h>
#include "../command.h"
#include "vmap.h"
#include "asm/pgtable.h"
#include "string.h"
#include "mm.h"
#include "virt.h"
#include "../virt/machine.h"

static void Usage(void)
{
	print("Usage: page_tlb_test [cmd]\n");
	print("cmd option:\n");
	print("    -- Acc (page table access bit test)\n");
	print("    -- Lazy (demanding page allocating test)\n");
	print("    -- sfence.vma_all (sfence.vma test)\n");
	print("    -- sfence.gvma_all (sfence.gvma test)\n");
	print("    -- remapping_gstage_memory_test\n");
}

static void page_table_remapping_gstage_memory_test()
{
	struct vcpu *vcpu;
	unsigned long gpa;
	unsigned int size;
	unsigned long hpa, hva;

	vcpu = vcpu_create();
	if (!vcpu)
		return;

	gpa = machine_get_memory_test_start(&vcpu->machine);
	size = machine_get_memory_test_size(&vcpu->machine);

	hva = (unsigned long)mm_alloc(size);
	if (!hva) {
		print("%s -- Out of memory\n", __FUNCTION__);
		return;
	}
	hpa = virt_to_phy(hva);

	strcpy((char *)hva, "Hello this is memory test pa2!!!");
	gstage_page_mapping((unsigned long *)vcpu->machine.gstage_pgdp, hpa,
			    gpa, size);

	mm_free((void *)hva, size);
}

static void page_table_sfence_gvma_all_test()
{
	struct vcpu *vcpu;

	vcpu = vcpu_create();
	if (!vcpu)
		return;

	print("Now, fence.gvma all...\n");
	vcpu_set_request(vcpu, VCPU_REQ_FENCE_GVMA_ALL);
}

static void page_table_sfence_all_test()
{
	pgprot_t pgprot;
	void *addr;
	void *va, *pa1, *pa2;
	int i = 0;

	addr = mm_alloc(PAGE_SIZE);
	if (!addr) {
		print("%s -- Out of memory\n", __FUNCTION__);
		goto ret;
	}
	pa1 = (void *)virt_to_phy(addr);
	strcpy((char *)addr, "sfence test -- This is pa1");
	print("print value in 0x%lx(pa1): %s\n", pa1, (char *)addr);

	addr = mm_alloc(PAGE_SIZE);
	if (!addr) {
		print("%s -- Out of memory\n", __FUNCTION__);
		goto ret2;
	}
	pa2 = (void *)virt_to_phy(addr);
	strcpy((char *)addr, "sfence test -- This is pa2");
	print("print value in 0x%lx(pa2): %s\n", pa2, (char *)addr);

	va = vmap_alloc(PAGE_SIZE);
	if (!va) {
		print("%s -- Out of memory\n", __FUNCTION__);
		goto ret3;
	}

	print("pa1:0x%lx pa2:0x%lx va:0x%lx\n", pa1, pa2, va);

	pgprot = __pgprot(_PAGE_BASE | _PAGE_READ | _PAGE_WRITE | _PAGE_DIRTY);

	print("Now map 0x%lx(pa) to 0x%lx(va)\n", pa1, va);
	if (-1 ==
	    mmu_page_mapping((unsigned long)pa1, (unsigned long)va, PAGE_SIZE,
			     pgprot)) {
		print("%s -- page mapping failed\n", __FUNCTION__);
		goto ret4;
	}
	print("load 0x%lx(--> 0x%lx)(make entry into tlb): %s\n", va, pa1, va);

	print("Now map 0x%lx(pa) to the same va(0x%lx)\n", pa2, va);
	if (-1 ==
	    mmu_page_mapping((unsigned long)pa2, (unsigned long)va, PAGE_SIZE,
			     pgprot)) {
		print("%s -- page mapping failed\n", __FUNCTION__);
		goto ret4;
	}

	print("test start --> load 0x%lx(before sfence.vma)\n", va);
	for (i = 0; i < 5; i++) {
		print("0x%lx : %s\n", va, (char *)va);
	}

	__asm__ __volatile("sfence.vma":::"memory");
	print("test start --> load 0x%lx(after sfence.vma)\n", va);
	for (i = 0; i < 5; i++) {
		print("0x%lx : %s\n", va, (char *)va);
	}
ret4:
	vmap_free(va, PAGE_SIZE);
ret3:
	mm_free((void *)phy_to_virt(pa2), PAGE_SIZE);
ret2:
	mm_free((void *)phy_to_virt(pa1), PAGE_SIZE);
ret:
	return;
}

static void page_table_access_bit_test()
{
	void *vaddr;
	unsigned long *pte;
	unsigned long pte_val;

	vaddr = vmem_alloc(PAGE_SIZE, NULL);
	if (!vaddr)
		return;

	print("vaddr:0x%lx\n", vaddr);
	strcpy(vaddr, "Hello");

	pte = mmu_get_pte((unsigned long)vaddr);
	if (!pte)
		return;
	pte_val = *pte;

	print("pte:0x%lx pte_val:0x%lx\n", pte, pte_val);
	print("%s\n", vaddr);

	pte_val &= ~(1UL << 6);	//Access bit
	print("remove access bit -- pte_val:0x%lx\n", pte_val);
	*pte = pte_val;

	__asm__ __volatile("sfence.vma":::"memory");

	print("%s\n", vaddr);

	vmem_free(vaddr, PAGE_SIZE);
}

static void page_demanding_allocating_test()
{
	char *vaddr;

	vaddr = (char *)vmem_alloc_lazy(PAGE_SIZE, NULL);
	if (!vaddr)
		return;

	print("vaddr:0x%lx\n", vaddr);

	strcpy(vaddr, "Hello");
	print("%s\n", vaddr);
}

static int cmd_page_tlb_test_handler(int argc, char *argv[], void *priv)
{
	if (argc != 1) {
		print("Invalid input params\n");
		Usage();
		return -1;
	}

	if (!strncmp(argv[0], "Lazy", sizeof("Lazy"))) {
		page_demanding_allocating_test();
	} else if (!strncmp(argv[0], "Acc", sizeof("Acc"))) {
		page_table_access_bit_test();
	} else if (!strncmp(argv[0], "sfence.vma_all", sizeof("sfence.vma_all"))) {
		page_table_sfence_all_test();
	} else if (!strncmp(argv[0], "sfence.gvma_all", sizeof("sfence.gvma_all"))) {
		page_table_sfence_gvma_all_test();
	} else if (!strncmp(argv[0], "remapping_gstage_memory_test", sizeof("remapping_gstage_memory_test"))) {
		page_table_remapping_gstage_memory_test();
	} else {
		print("Unsupport command\n");
		Usage();
		return -1;
	}

	return 0;
}

static const struct command cmd_page_tlb_test = {
	.cmd = "page_tlb_test",
	.handler = cmd_page_tlb_test_handler,
	.priv = NULL,
};

int page_tlb_test_init()
{
	register_command(&cmd_page_tlb_test);

	return 0;
}

APP_COMMAND_REGISTER(page_tlb_test, page_tlb_test_init);
